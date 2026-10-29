#include "base/include/arena.h"
#include "base/include/list.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  union {
    double e[3];
    struct {
      double x;
      double y;
      double z;
    };
  };
} Vertex;

typedef struct {
  List(int) indices;

} Face;

typedef struct {
  List(Vertex) vertices;
  List(Face) faces;
  size_t size;
} ParsedObj;

static inline ParsedObj parsed_obj_new(Arena *arena) {
  ParsedObj result = (ParsedObj){0};
  list_init(&result.faces, arena);
  list_init(&result.vertices, arena);

  return result;
}

static inline int is_obj_whitespace(char c) {
  return c == ' ' || c == '\t' || c == '\r';
}

static inline Vertex parse_vertex_line(char *current) {
  Vertex result = {0};
  char *cursor = current;

  while (is_obj_whitespace(*cursor)) {
    cursor++;
  }
  if (*cursor == 'v') {
    cursor++;
  }

  result.x = strtod(cursor, &cursor);
  result.y = strtod(cursor, &cursor);
  result.z = strtod(cursor, &cursor);

  return result;
}

static inline void
parse_face_line(char *current, ParsedObj *result, Arena *arena) {
  Face face = {0};
  list_init(&face.indices, arena);

  char *cursor = current;
  while (is_obj_whitespace(*cursor)) {
    cursor++;
  }
  if (*cursor == 'f') {
    cursor++;
  }

  while (*cursor != '\0' && *cursor != '#') {
    while (is_obj_whitespace(*cursor)) {
      cursor++;
    }
    if (*cursor == '\0' || *cursor == '#') {
      break;
    }

    char *end = cursor;
    long obj_index = strtol(cursor, &end, 10);
    if (end == cursor) {
      break;
    }
    cursor = end;

    long vertex_index = obj_index;
    if (obj_index < 0) {
      vertex_index = (long)result->vertices.count + obj_index;
    } else if (obj_index > 0) {
      vertex_index = obj_index - 1;
    }

    if (obj_index != 0 && vertex_index >= 0 &&
        (size_t)vertex_index < result->vertices.count) {
      int index = (int)vertex_index;
      list_push(&face.indices, index);
    }

    // Skip over the optional texture/normal components (v/vt/vn).
    while (*cursor != '\0' && !is_obj_whitespace(*cursor)) {
      cursor++;
    }
  }

  if (face.indices.count >= 3) {
    list_push(&result->faces, face);
  }
}

static inline ParsedObj parse_obj_file(char *data, Arena *arena) {
  ParsedObj result = parsed_obj_new(arena);
  if (data == NULL) {
    return result;
  }

  char *current_line = data;
  while (*current_line != '\0') {
    char *next_line = strchr(current_line, '\n');
    size_t len =
        next_line ? (size_t)(next_line - current_line) : strlen(current_line);

    char line[len + 1];
    memcpy(line, current_line, len);
    line[len] = '\0';

    char *cursor = line;
    while (is_obj_whitespace(*cursor)) {
      cursor++;
    }

    if (cursor[0] == 'v' && is_obj_whitespace(cursor[1])) {
      Vertex vertex = parse_vertex_line(cursor);
      list_push(&result.vertices, vertex);
    } else if (cursor[0] == 'f' && is_obj_whitespace(cursor[1])) {
      parse_face_line(cursor, &result, arena);
    }
    // Other OBJ directives (vn, vt, o, g, usemtl, ...) are ignored.

    if (next_line == NULL) {
      break;
    }
    current_line = next_line + 1;
  }

  result.size = result.vertices.count;
  return result;
}

static inline ParsedObj read_file(const char *path, Arena *arena) {
  FILE *f = fopen(path, "rb");
  if (!f) {
    return (ParsedObj){0};
  }
  fseek(f, 0, SEEK_END);
  size_t size = (size_t)ftell(f);
  fseek(f, 0, SEEK_SET);
  ArenaTemp temp_arena = arena_temp_get(&arena, 1);
  char *data = (char *)arena_push(temp_arena.arena, size + 1);
  fread(data, 1, size, f);
  data[size] = '\0';
  fclose(f);
  ParsedObj parsed_obj = parse_obj_file(data, arena);
  arena_temp_release(temp_arena);

  return parsed_obj;
}

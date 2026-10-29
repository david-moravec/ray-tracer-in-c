#include "../base/utest/utest.h"
#include "../obj_parser.h"

#include <stdbool.h>
#include <stdlib.h>

// Test-only arena backing. This keeps the parser tests independent from the
// OS-specific memory implementation in base/memory.
void *platform_memory_allocate_block(size_t reserved_size, size_t alignment) {
  (void)alignment;
  return calloc(1, reserved_size);
}

size_t platform_memory_default_page_size(void) { return 16; }

bool platform_memory_check_to_commit_protect(void *block, size_t to_commit) {
  (void)block;
  (void)to_commit;
  return true;
}

_Thread_local Arena *tls_arenas[2];

static int nearly_equal(double a, double b) {
  double diff = a - b;
  if (diff < 0) {
    diff = -diff;
  }
  return diff < 1e-9;
}

static int vertex_is(Vertex *vertex, double x, double y, double z) {
  return nearly_equal(vertex->x, x) && nearly_equal(vertex->y, y) &&
         nearly_equal(vertex->z, z);
}

static int face_is(Face *face, const int *expected, size_t count) {
  if (face->indices.count != count) {
    return 0;
  }

  for (size_t i = 0; i < count; i++) {
    if (*list_get(&face->indices, i) != expected[i]) {
      return 0;
    }
  }

  return 1;
}

TEST(test_empty_and_null_input) {
  Arena *arena = arena_alloc(1 << 20, 0);

  ParsedObj null_obj = parse_obj_file(NULL, arena);
  ASSERT(null_obj.vertices.count == 0, "NULL input has no vertices");
  ASSERT(null_obj.faces.count == 0, "NULL input has no faces");
  ASSERT(null_obj.size == 0, "NULL input has zero size");

  char empty_obj[] = "";
  ParsedObj empty = parse_obj_file(empty_obj, arena);
  ASSERT(empty.vertices.count == 0, "empty input has no vertices");
  ASSERT(empty.faces.count == 0, "empty input has no faces");

  char comments_obj[] = "# a comment\n"
                        "\n"
                        "   \n"
                        "vt 0 0\n"
                        "vn 0 0 1\n"
                        "o object\n"
                        "g group\n"
                        "usemtl material\n";
  ParsedObj comments = parse_obj_file(comments_obj, arena);
  ASSERT(comments.vertices.count == 0, "ignored directives add no vertices");
  ASSERT(comments.faces.count == 0, "ignored directives add no faces");

  arena_release(arena);
}

TEST(test_parse_vertex_line) {
  char line[] = "v 1.5 -2 3.25";
  Vertex vertex = parse_vertex_line(line);
  ASSERT(vertex_is(&vertex, 1.5, -2.0, 3.25), "parses x, y and z");

  char indented[] = "   v\t4 5 6";
  Vertex indented_vertex = parse_vertex_line(indented);
  ASSERT(vertex_is(&indented_vertex, 4.0, 5.0, 6.0),
         "accepts leading whitespace and tabs");

  char no_coordinates[] = "v";
  Vertex zero_vertex = parse_vertex_line(no_coordinates);
  ASSERT(vertex_is(&zero_vertex, 0.0, 0.0, 0.0),
         "missing coordinates default to zero");
}

TEST(test_parse_vertices) {
  Arena *arena = arena_alloc(1 << 20, 0);
  char obj[] = "v 1 2 3\n"
               "  v\t4 5.5 -6\n"
               "v -1.25 2.5 0\r\n"
               "v 7 8 9\n";

  ParsedObj parsed = parse_obj_file(obj, arena);

  ASSERT(parsed.vertices.count == 4, "parses all vertex lines");
  ASSERT(parsed.faces.count == 0, "vertex-only file has no faces");
  ASSERT(parsed.size == 4, "size is the vertex count");

  ASSERT(vertex_is(list_get(&parsed.vertices, 0), 1.0, 2.0, 3.0),
         "first vertex");
  ASSERT(vertex_is(list_get(&parsed.vertices, 1), 4.0, 5.5, -6.0),
         "second vertex with tabs");
  ASSERT(vertex_is(list_get(&parsed.vertices, 2), -1.25, 2.5, 0.0),
         "third vertex with CRLF line ending");
  ASSERT(vertex_is(list_get(&parsed.vertices, 3), 7.0, 8.0, 9.0),
         "fourth vertex");

  Vertex *first = list_get(&parsed.vertices, 0);
  ASSERT(first->e[0] == first->x, "e[0] aliases x");
  ASSERT(first->e[1] == first->y, "e[1] aliases y");
  ASSERT(first->e[2] == first->z, "e[2] aliases z");

  arena_release(arena);
}

TEST(test_face_index_formats) {
  Arena *arena = arena_alloc(1 << 20, 0);
  char obj[] = "v 0 0 0\n"
               "v 1 0 0\n"
               "v 0 1 0\n"
               "f 1 2 3\n"
               "f 1/1 2/2 3/3\n"
               "f 1//1 2//2 3//3\n"
               "f 1/1/1 2/2/2 3/3/3\n";

  ParsedObj parsed = parse_obj_file(obj, arena);
  int expected[] = {0, 1, 2};

  ASSERT(parsed.faces.count == 4, "parses every face index format");
  for (size_t i = 0; i < parsed.faces.count; i++) {
    ASSERT(face_is(list_get(&parsed.faces, i), expected, 3),
           "face indices are converted to zero-based");
  }

  arena_release(arena);
}

TEST(test_polygon_face) {
  Arena *arena = arena_alloc(1 << 20, 0);
  char obj[] = "v 0 0 0\n"
               "v 1 0 0\n"
               "v 1 1 0\n"
               "v 0 1 0\n"
               "f 1 2 3 4\n";

  ParsedObj parsed = parse_obj_file(obj, arena);
  int expected[] = {0, 1, 2, 3};

  ASSERT(parsed.faces.count == 1, "parses a polygon face");
  ASSERT(face_is(list_get(&parsed.faces, 0), expected, 4),
         "keeps all polygon indices in order");

  arena_release(arena);
}

TEST(test_negative_face_indices) {
  Arena *arena = arena_alloc(1 << 20, 0);
  char obj[] = "v 0 0 0\n"
               "v 1 0 0\n"
               "v 0 1 0\n"
               "v 1 1 0\n"
               "f -4 -3 -2\n"
               "f -2 -1 -4\n";

  ParsedObj parsed = parse_obj_file(obj, arena);
  int first_expected[] = {0, 1, 2};
  int second_expected[] = {2, 3, 0};

  ASSERT(parsed.faces.count == 2, "parses negative face indices");
  ASSERT(face_is(list_get(&parsed.faces, 0), first_expected, 3),
         "resolves negative indices from the start of the vertex list");
  ASSERT(face_is(list_get(&parsed.faces, 1), second_expected, 3),
         "resolves negative indices relative to the current vertex count");

  arena_release(arena);
}

TEST(test_invalid_faces_and_comments) {
  Arena *arena = arena_alloc(1 << 20, 0);
  char obj[] = "v 0 0 0\n"
               "v 1 0 0\n"
               "v 0 1 0\n"
               "v 1 1 0\n"
               "f 1 2 99\n"
               "f 1 2\n"
               "f 0 1 2\n"
               "f 1 2 3 # trailing comment\n"
               "f 1 2 3 4 99\n";

  ParsedObj parsed = parse_obj_file(obj, arena);
  int triangle_expected[] = {0, 1, 2};
  int polygon_expected[] = {0, 1, 2, 3};

  ASSERT(parsed.faces.count == 2,
         "faces with fewer than three valid indices are skipped");
  ASSERT(face_is(list_get(&parsed.faces, 0), triangle_expected, 3),
         "ignores a trailing comment");
  ASSERT(face_is(list_get(&parsed.faces, 1), polygon_expected, 4),
         "ignores invalid indices inside a face");

  arena_release(arena);
}

TEST(test_face_on_last_line_without_newline) {
  Arena *arena = arena_alloc(1 << 20, 0);
  char obj[] = "v 0 0 0\n"
               "v 1 0 0\n"
               "v 0 1 0\n"
               "f 1 2 3";
  int expected[] = {0, 1, 2};

  ParsedObj parsed = parse_obj_file(obj, arena);

  ASSERT(parsed.vertices.count == 3, "parses vertices before the last line");
  ASSERT(parsed.faces.count == 1, "parses a last line without a newline");
  ASSERT(face_is(list_get(&parsed.faces, 0), expected, 3),
         "last-line face indices are correct");

  arena_release(arena);
}

int main(void) {
  RUN_TEST(test_empty_and_null_input);
  RUN_TEST(test_parse_vertex_line);
  RUN_TEST(test_parse_vertices);
  RUN_TEST(test_face_index_formats);
  RUN_TEST(test_polygon_face);
  RUN_TEST(test_negative_face_indices);
  RUN_TEST(test_invalid_faces_and_comments);
  RUN_TEST(test_face_on_last_line_without_newline);
  SUMMARY();
}

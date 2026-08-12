#ifndef RAYTRACER_TEXTURE_C
#define RAYTRACER_TEXTURE_C

#include "base/include/arena.h"
#include "color.c"

typedef enum {
  RAYTRACER_TEXTURE_TYPE_SOLID_COLOR,
  RAYTRACER_TEXTURE_TYPE_CHECKERED,
} TextureType;

typedef struct _Texture {
  TextureType type;

  union {
    // Solid color
    Color albedo;

    // Checkered
    struct {
      double inv_scale;
      struct _Texture *even;
      struct _Texture *odd;
    };
  };
} Texture;

Color texture_value(Texture *texture, double u, double v, Point3 p);

// Solid

Texture texture_solid_color_from_color(Color color) {
  return (Texture){.type = RAYTRACER_TEXTURE_TYPE_SOLID_COLOR, .albedo = color};
}

Texture texture_solid_color_from_rgb(double red, double green, double blue) {
  return (Texture){.type = RAYTRACER_TEXTURE_TYPE_SOLID_COLOR,
                   .albedo = color_new(red, green, blue)};
}

Color texture_value_solid_color(Texture *texture, double u, double v,
                                Point3 p) {
  return texture->albedo;
}

// Checkered

Texture texture_checkered_from_textures(double scale, Texture *even,
                                        Texture *odd) {
  return (Texture){.type = RAYTRACER_TEXTURE_TYPE_CHECKERED,
                   .inv_scale = (1.0 / scale),
                   .even = even,
                   .odd = odd};
}

Texture texture_checkered_from_colors(double scale, Color c1, Color c2,
                                      Arena *arena) {
  Texture solid_c1 = texture_solid_color_from_color(c1);
  Texture solid_c2 = texture_solid_color_from_color(c2);

  return (Texture){.type = RAYTRACER_TEXTURE_TYPE_CHECKERED,
                   .inv_scale = (1.0 / scale),
                   .even = ARENA_PUSH_COPY(arena, Texture, &solid_c1),
                   .odd = ARENA_PUSH_COPY(arena, Texture, &solid_c2)};
}

Color texture_value_checkered(Texture *texture, double u, double v, Point3 p) {
  int x_int = (int)(floor(texture->inv_scale * p.x));
  int y_int = (int)(floor(texture->inv_scale * p.y));
  int z_int = (int)(floor(texture->inv_scale * p.z));

  bool is_even = (x_int + y_int + z_int) % 2 == 0;

  return is_even ? texture_value(texture->even, u, v, p)
                 : texture_value(texture->odd, u, v, p);
}

Color texture_value(Texture *texture, double u, double v, Point3 p) {
  switch (texture->type) {
  case RAYTRACER_TEXTURE_TYPE_SOLID_COLOR: {
    return texture_value_solid_color(texture, u, v, p);
  }
  case RAYTRACER_TEXTURE_TYPE_CHECKERED: {
    return texture_value_checkered(texture, u, v, p);
  }
  }
}

#endif

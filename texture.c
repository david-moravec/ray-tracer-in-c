#ifndef RAYTRACER_TEXTURE_C
#define RAYTRACER_TEXTURE_C

#include "base/include/arena.h"
#include "color.c"
#include "interval.c"
#include "perlin.c"
#include "rtc_stb_image.h"
#include "vec3.c"

typedef enum {
  RAYTRACER_TEXTURE_TYPE_SOLID_COLOR,
  RAYTRACER_TEXTURE_TYPE_CHECKERED,
  RAYTRACER_TEXTURE_TYPE_IMAGE,
  RAYTRACER_TEXTURE_TYPE_NOISE,
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

    // Image
    RTCImage *image;

    // Noise
    struct {
      Perlin noise;
      double scale;
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

// Image

Texture texture_image_new(const char *path, Arena *arena) {
  RTCImage image = rtc_image_new(path, arena);

  return (Texture){.type = RAYTRACER_TEXTURE_TYPE_IMAGE,
                   .image = ARENA_PUSH_COPY(arena, RTCImage, &image)};
}

Color texture_value_image(Texture *texture, double u, double v, Point3 p) {
  RTCImage *image = texture->image;
  if (image->height <= 0) {
    return color_new(0, 1, 1);
  }

  u = clamp_value(0, 1, u);
  v = 1.0 - clamp_value(0, 1, v);

  int i = (int)(u * image->width);
  int j = (int)(v * image->height);
  const unsigned char *pixel = rtc_image_pixel_data(image, i, j);

  double color_scale = 1.0 / 255.0;

  return color_new(color_scale * pixel[0], color_scale * pixel[1],
                   color_scale * pixel[2]);
}

// Noise

Texture texture_noise_new(double scale) {
  return (Texture){.type = RAYTRACER_TEXTURE_TYPE_NOISE,
                   .noise = perlin_new(),
                   .scale = scale};
}

Color texture_value_noise(Texture *texture, double u, double v, Point3 p) {
  return vec3_scalar_multiply(
      color_new(1, 1, 1),
      perlin_noise(&texture->noise, vec3_scalar_multiply(p, texture->scale)));
}
// Static dispatch

Color texture_value(Texture *texture, double u, double v, Point3 p) {
  switch (texture->type) {
  case RAYTRACER_TEXTURE_TYPE_SOLID_COLOR: {
    return texture_value_solid_color(texture, u, v, p);
  }
  case RAYTRACER_TEXTURE_TYPE_CHECKERED: {
    return texture_value_checkered(texture, u, v, p);
  }
  case RAYTRACER_TEXTURE_TYPE_IMAGE: {
    return texture_value_image(texture, u, v, p);
  }
  case RAYTRACER_TEXTURE_TYPE_NOISE: {
    return texture_value_noise(texture, u, v, p);
  }
  }
}

#endif

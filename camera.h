#ifndef RAYTRACER_CAMERA_H
#define RAYTRACER_CAMERA_H

#include "material.h"
#include "rtcommon.h"
#include "stdbool.h"
#include "stdio.h"

#include "color.h"
#include "hittable.h"
#include "ray.h"
#include "vec3.h"

typedef float f32;
typedef double f64;
typedef uint16_t u16;

typedef struct {
  double aspect_ratio;
  u16 image_width;
  u16 samples_per_pixel;
  u16 max_depth;

  // assigned during initialize
  u16 image_height;
  double pixel_samples_scale;
  Point3 center;
  Point3 top_left_pixel_loc;
  Vec3 pixel_delta_u;
  Vec3 pixel_delta_v;
} Camera;

static inline Color ray_color(const Ray ray, u16 depth, const Hittable *world) {
  if (depth == 0) {
    return color_new(0.0, 0.0, 0.0);
  }
  HitRecord record;
  if (hittable_hit(world, ray, interval_new(0.001, INFINITY), &record)) {
    Ray scattered = {0};
    Color attenuation = {0};
    if (material_scatter(record.material, ray, record, &attenuation,
                         &scattered)) {
      return vec3_multiply(attenuation, ray_color(scattered, depth - 1, world));
    }
    return (Color){0};
  }

  Vec3 unit_direction = vec3_unit_vector(ray.direction);
  double a = 0.5 * (unit_direction.y + 1.0);
  Color blue = color_new(0.6, 0.7, 1.0);
  Color white = color_new(1.0, 1.0, 1.0);

  return vec3_add(vec3_scalar_multiply(white, (1.0 - a)),
                  vec3_scalar_multiply(blue, a));
}

static inline void camera_initialize(Camera *camera) {
  u16 image_width = 400;
  u16 image_height = (u16)(image_width / camera->aspect_ratio);
  camera->image_height = (image_height < 1) ? 1 : image_height;

  camera->pixel_samples_scale = 1.0 / camera->samples_per_pixel;

  f64 focal_length = 1.0;
  f64 viewport_height = 2.0;
  f64 viewport_width = viewport_height *
                       ((f64)(camera->image_width) / (f64)camera->image_height);
  Point3 camera_center = point3_new(0.0, 0.0, 0.0);

  Vec3 viewport_u = vec3_new(viewport_width, 0, 0);
  Vec3 viewport_v = vec3_new(0, -viewport_height, 0);

  camera->pixel_delta_u = vec3_scalar_devide(viewport_u, image_width);
  camera->pixel_delta_v = vec3_scalar_devide(viewport_v, image_height);

  Vec3 viewport_center =
      vec3_subtract(camera_center, vec3_new(0.0, 0.0, focal_length));
  Vec3 upper_left_offset = vec3_add(vec3_scalar_devide(viewport_u, 2),
                                    vec3_scalar_devide(viewport_v, 2));
  Vec3 viewport_upper_left = vec3_subtract(viewport_center, upper_left_offset);
  camera->top_left_pixel_loc = vec3_add(
      viewport_upper_left,
      vec3_scalar_multiply(
          vec3_add(camera->pixel_delta_u, camera->pixel_delta_v), 0.5));
}

static inline Vec3 sample_square() {
  return vec3_new(random_double() - 0.5, random_double() - 0.5, 0);
}

static inline Ray get_ray(Camera *camera, u16 i, u16 j) {
  Vec3 offset = sample_square();
  Vec3 pixel_offset =
      vec3_add(vec3_scalar_multiply(camera->pixel_delta_u, i + offset.x),
               vec3_scalar_multiply(camera->pixel_delta_v, j + offset.y));

  Vec3 pixel_sample = vec3_add(camera->top_left_pixel_loc, pixel_offset);

  Point3 ray_origin = camera->center;
  Vec3 ray_direction = vec3_subtract(pixel_sample, ray_origin);

  return ray_new(ray_origin, ray_direction);
}

static inline void camera_render(Camera *camera, Hittable *world) {
  char buff[BUFSIZ];
  setvbuf(stderr, buff, _IOFBF, BUFSIZ);
  printf("P3\n%u %u\n255\n", camera->image_width, camera->image_height);

  for (int j = 0; j < camera->image_height; j++) {
    fprintf(stderr, "\rScanlines reamining: %-6u", (camera->image_height - j));
    fflush(stderr);
    for (int i = 0; i < camera->image_width; i++) {
      Color pixel_color = {0};
      for (int sample = 0; sample < camera->samples_per_pixel; sample++) {
        Ray ray = get_ray(camera, i, j);
        pixel_color =
            vec3_add(pixel_color, ray_color(ray, camera->max_depth, world));
      }

      color_fprint(stdout, vec3_scalar_multiply(pixel_color,
                                                camera->pixel_samples_scale));
    }
  }
  fprintf(stderr, "\r%-40s\n", "Done");
  fflush(stderr);
}

#endif

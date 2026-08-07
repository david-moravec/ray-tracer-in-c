#ifndef RAYTRACER_CAMERA_C
#define RAYTRACER_CAMERA_C

#include "base/include/arena.h"
#include "stdbool.h"
#include "stdio.h"
#include "string.h"

#include "rtcommon.h"

#include "color.c"
#include "hittable.c"
#include "material.c"
#include "ray.c"
#include "vec3.c"

typedef float f32;
typedef double f64;
typedef uint16_t u16;

typedef struct {
  double aspect_ratio;
  u16 image_width;
  u16 samples_per_pixel;
  u16 max_depth;

  double vfov;
  Point3 look_from;
  Point3 look_at;
  Vec3 vup;

  double defocus_angle;
  double focus_dist;

  // assigned during initialize
  u16 image_height;
  double pixel_samples_scale;
  Point3 center;
  Point3 top_left_pixel_loc;
  Vec3 pixel_delta_u;
  Vec3 pixel_delta_v;
  Vec3 u, v, w;
  Vec3 defocus_disk_u;
  Vec3 defocus_disk_v;
} Camera;

Color ray_color(const Ray ray, u16 depth, const Hittable *world) {
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

void camera_initialize(Camera *camera) {
  u16 image_height = (u16)(camera->image_width / camera->aspect_ratio);
  camera->image_height = (image_height < 1) ? 1 : image_height;

  camera->pixel_samples_scale = 1.0 / camera->samples_per_pixel;

  camera->center = camera->look_from;
  Vec3 look_direction = vec3_subtract(camera->look_from, camera->look_at);

  f64 theta = deg_to_rad(camera->vfov);
  f64 h = tan(theta / 2.0);
  f64 viewport_height = 2.0 * h * camera->focus_dist;
  f64 viewport_width = viewport_height *
                       ((f64)(camera->image_width) / (f64)camera->image_height);

  camera->w = vec3_unit_vector(look_direction);
  camera->u = vec3_unit_vector(vec3_cross_product(camera->vup, camera->w));
  camera->v = vec3_cross_product(camera->w, camera->u);

  Vec3 viewport_u = vec3_scalar_multiply(camera->u, viewport_width);
  Vec3 viewport_v =
      vec3_scalar_multiply(vec3_negative(camera->v), viewport_height);

  camera->pixel_delta_u = vec3_scalar_devide(viewport_u, camera->image_width);
  camera->pixel_delta_v = vec3_scalar_devide(viewport_v, image_height);

  Vec3 viewport_center = vec3_subtract(
      camera->center, vec3_scalar_multiply(camera->w, camera->focus_dist));

  Vec3 upper_left_offset = vec3_add(vec3_scalar_devide(viewport_u, 2),
                                    vec3_scalar_devide(viewport_v, 2));
  Vec3 viewport_upper_left = vec3_subtract(viewport_center, upper_left_offset);
  camera->top_left_pixel_loc = vec3_add(
      viewport_upper_left,
      vec3_scalar_multiply(
          vec3_add(camera->pixel_delta_u, camera->pixel_delta_v), 0.5));

  double defocus_radius =
      camera->focus_dist * tan(deg_to_rad(camera->defocus_angle / 2.0));
  camera->defocus_disk_u = vec3_scalar_multiply(camera->u, defocus_radius);
  camera->defocus_disk_v = vec3_scalar_multiply(camera->v, defocus_radius);
}

Vec3 sample_square() {
  return vec3_new(random_double() - 0.5, random_double() - 0.5, 0);
}

static inline Point3 defocus_disk_sample(Camera *camera) {
  Vec3 p = vec3_random_in_unit_disk();

  return vec3_add(camera->center,
                  vec3_add(vec3_scalar_multiply(camera->defocus_disk_u, p.x),
                           vec3_scalar_multiply(camera->defocus_disk_v, p.y)));
}

Ray get_ray(Camera *camera, u16 i, u16 j) {
  Vec3 offset = sample_square();
  Vec3 pixel_offset =
      vec3_add(vec3_scalar_multiply(camera->pixel_delta_u, i + offset.x),
               vec3_scalar_multiply(camera->pixel_delta_v, j + offset.y));

  Vec3 pixel_sample = vec3_add(camera->top_left_pixel_loc, pixel_offset);

  Point3 ray_origin =
      camera->defocus_angle <= 0 ? camera->center : defocus_disk_sample(camera);
  Vec3 ray_direction = vec3_subtract(pixel_sample, ray_origin);

  return ray_new(ray_origin, ray_direction);
}

void camera_render(Camera *camera, Hittable *world, Arena *arena) {
  char buff[BUFSIZ];
  setvbuf(stderr, buff, _IOFBF, BUFSIZ);
  printf("P3\n%u %u\n255\n", camera->image_width, camera->image_height);

  uint32_t pixel_count = camera->image_height * camera->image_width;

  Color *frame_buff = (Color *)arena_push(arena, pixel_count * sizeof(Color));

  for (int y = 0; y < camera->image_height; y++) {
    fprintf(stderr, "\rScanlines reamining: %-6u", (camera->image_height - y));
    fflush(stderr);
    for (int x = 0; x < camera->image_width; x++) {
      Color pixel_color = {0};
      for (int sample = 0; sample < camera->samples_per_pixel; sample++) {
        Ray ray = get_ray(camera, x, y);
        pixel_color =
            vec3_add(pixel_color, ray_color(ray, camera->max_depth, world));
      }
      pixel_color =
          vec3_scalar_multiply(pixel_color, camera->pixel_samples_scale);

      frame_buff[y * camera->image_width + x] = pixel_color;
    }
  }

  for (int i = 0; i < pixel_count; i++) {
    color_fprint(stdout, frame_buff[i]);
  }
  fprintf(stderr, "\r%-40s\n", "Done");
  fflush(stderr);
}

#endif

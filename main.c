#include "base/include/arena.h"
#include "color.h"
#include "stdbool.h"
#include "stdint.h"
#include "stdio.h"

#include "hittable.h"
#include "vec3.h"

typedef float f32;
typedef double f64;
typedef uint16_t u16;

const double ASPECT_RATIO = 16.0 / 9.0;

Color ray_color(const Ray ray, const Hittable *world) {
  HitRecord record;
  if (hittable_hit(world, ray, 0, INFINITY, &record)) {
    return vec3_scalar_multiply(
        vec3_add(color_new(1.0, 1.0, 1.0), record.normal), 0.5);
  }

  Vec3 unit_direction = vec3_unit_vector(ray.direction);
  double a = 0.5 * (unit_direction.y + 1.0);
  Color blue = color_new(0.6, 0.7, 1.0);
  Color white = color_new(1.0, 1.0, 1.0);

  return vec3_add(vec3_scalar_multiply(white, (1.0 - a)),
                  vec3_scalar_multiply(blue, a));
}

int main() {
  Arena *arena = arena_alloc(1 << 20, 0);
  char buff[BUFSIZ];
  setvbuf(stderr, buff, _IOFBF, BUFSIZ);

  u16 image_width = 400;
  u16 image_height = (u16)(image_width / ASPECT_RATIO);
  image_height = (image_height < 1) ? 1 : image_height;

  Hittable world = hittable_collection_new(arena);

  hittable_list_add((HittableList *)world.collection,
                    hittable_sphere_new(point3_new(0.0, 0.0, -1.0), 0.5));

  hittable_list_add((HittableList *)world.collection,
                    hittable_sphere_new(point3_new(0.0, -100.5, -1.0), 100));

  f64 focal_length = 1.0;
  f64 viewport_height = 2.0;
  f64 viewport_width =
      viewport_height * ((f64)(image_width) / (f64)image_height);
  Point3 camera_center = point3_new(0.0, 0.0, 0.0);

  Vec3 viewport_u = vec3_new(viewport_width, 0, 0);
  Vec3 viewport_v = vec3_new(0, -viewport_height, 0);

  Vec3 pixel_delta_u = vec3_scalar_devide(viewport_u, image_width);
  Vec3 pixel_delta_v = vec3_scalar_devide(viewport_v, image_height);

  Vec3 viewport_center =
      vec3_subtract(camera_center, vec3_new(0.0, 0.0, focal_length));
  Vec3 upper_left_offset = vec3_add(vec3_scalar_devide(viewport_u, 2),
                                    vec3_scalar_devide(viewport_v, 2));
  Vec3 viewport_upper_left = vec3_subtract(viewport_center, upper_left_offset);
  Point3 top_left_pixel_loc = vec3_add(
      viewport_upper_left,
      vec3_scalar_multiply(vec3_add(pixel_delta_u, pixel_delta_v), 0.5));

  printf("P3\n%u %u\n255\n", image_width, image_height);

  for (int j = 0; j < image_height; j++) {
    fprintf(stderr, "\rScanlines reamining: %-6u", (image_height - j));
    fflush(stderr);
    for (int i = 0; i < image_width; i++) {
      Vec3 pixel_offset = vec3_add(vec3_scalar_multiply(pixel_delta_u, i),
                                   vec3_scalar_multiply(pixel_delta_v, j));
      Point3 pixel_location = vec3_add(top_left_pixel_loc, pixel_offset);
      const Ray r =
          ray_new(camera_center, vec3_subtract(pixel_location, camera_center));

      Color pixel_color = ray_color(r, &world);

      color_fprint(stdout, &pixel_color);
    }
  }
  fprintf(stderr, "\r%-40s\n", "Done");
  fflush(stderr);
}

#include "stdbool.h"
#include "stdint.h"
#include "stdio.h"

#include "color.h"
#include "math.h"
#include "ray.h"
#include "vec3.h"

typedef float f32;
typedef double f64;
typedef uint16_t u16;

const double ASPECT_RATIO = 16.0 / 9.0;

double ray_hit_sphere(const Ray ray, const Point3 center, double r) {
  Vec3 origin_to_center = vec3_subtract(center, ray.origin);

  double a = vec3_length_squared(ray.direction);
  double h = vec3_dot_product(ray.direction, origin_to_center);
  double c = vec3_length_squared(origin_to_center) - r * r;
  double discriminant = h * h - a * c;

  if (discriminant < 0) {
    return -1.0;
  } else {
    return (h - sqrt(discriminant)) / a;
  }
}

Color ray_color(const Ray r) {
  double t = ray_hit_sphere(r, (Point3){.x = 0.0, .y = 0.0, .z = -1.0}, 0.5);
  if (t > 0.0) {
    Vec3 n = vec3_unit_vector(
        vec3_subtract(ray_at(r, t), (Vec3){.x = 0, .y = 0, .z = -1}));
    return vec3_scalar_multiply(
        (Color){.x = n.x + 1.0, .y = n.y + 1, .z = n.z + 1}, 0.5);
  }

  Vec3 unit_direction = vec3_unit_vector(r.direction);
  double a = 0.5 * (unit_direction.y + 1.0);
  Color blue = {.x = 0.6, .y = 0.7, .z = 1.0};
  Color white = {.x = 1.0, .y = 1.0, .z = 1.0};

  return vec3_add(vec3_scalar_multiply(white, (1.0 - a)),
                  vec3_scalar_multiply(blue, a));
}

int main() {
  char buff[BUFSIZ];
  setvbuf(stderr, buff, _IOFBF, BUFSIZ);

  u16 image_width = 400;
  u16 image_height = (u16)(image_width / ASPECT_RATIO);
  image_height = (image_height < 1) ? 1 : image_height;

  f64 focal_length = 1.0;
  f64 viewport_height = 2.0;
  f64 viewport_width =
      viewport_height * ((f64)(image_width) / (f64)image_height);
  Point3 camera_center = {.x = 0, .y = 0, .z = 0};

  Vec3 viewport_u = {.x = viewport_width, .y = 0, .z = 0};
  Vec3 viewport_v = {.x = 0, .y = -viewport_height, .z = 0};

  Vec3 pixel_delta_u = vec3_scalar_devide(viewport_u, image_width);
  Vec3 pixel_delta_v = vec3_scalar_devide(viewport_v, image_height);

  Vec3 viewport_center =
      vec3_subtract(camera_center, (Vec3){.x = 0, .y = 0, .z = focal_length});
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
      const Ray r = {.origin = camera_center,
                     .direction = vec3_subtract(pixel_location, camera_center)};

      Color pixel_color = ray_color(r);

      color_fprint(stdout, &pixel_color);
    }
  }
  fprintf(stderr, "\r%-40s\n", "Done");
  fflush(stderr);
}

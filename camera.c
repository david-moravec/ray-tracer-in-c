#ifndef RAYTRACER_CAMERA_C
#define RAYTRACER_CAMERA_C

#include "stdbool.h"

#include "multihreading.c"
#include "rtcommon.h"

#include "color.c"
#include "hittable.c"
#include "material.c"
#include "ray.c"
#include "vec3.c"
#include <minwindef.h>

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

Color trace_pixel(Camera *camera, int x, int y, Hittable *world) {
  Color pixel_color = {0};
  for (int sample = 0; sample < camera->samples_per_pixel; sample++) {
    Ray ray = get_ray(camera, x, y);
    pixel_color =
        vec3_add(pixel_color, ray_color(ray, camera->max_depth, world));
  }
  return vec3_scalar_multiply(pixel_color, camera->pixel_samples_scale);
}

void trace_row(Camera *camera, int y, Hittable *world, Color *frame_buff) {
  for (int x = 0; x < camera->image_width; x++) {
    frame_buff[y * camera->image_width + x] = trace_pixel(camera, x, y, world);
  }
}

void _render(Camera *camera, Hittable *world, Color *frame_buff) {
  // NOTE make this depend on a compiler flag MULTITHREADED
  char buff[BUFSIZ];
  setvbuf(stderr, buff, _IOFBF, BUFSIZ);

  for (int y = 0; y < camera->image_height; y++) {
    fprintf(stderr, "\rScanlines reamining: %-6u", (camera->image_height - y));
    fflush(stderr);

    trace_row(camera, y, world, frame_buff);
  }

  fprintf(stderr, "\r%-40s\n", "Done");
  fflush(stderr);
}

void camera_render(Camera *camera, Hittable *world, Color *frame_buff) {
  _render(camera, world, frame_buff);
}

typedef struct {
  int thread_id;
  int number_of_threads;
  Camera *camera;
  Hittable *world;
  Color *frame_buff;

} ThreadArgs;

DWORD WINAPI _render_multithread(LPVOID param) {
  ThreadArgs *args = (ThreadArgs *)param;

  for (int y = args->thread_id; y < args->camera->image_height;
       y += args->number_of_threads) {
    for (int x = 0; x < args->camera->image_width; x++) {
      Color c = trace_pixel(args->camera, x, y, args->world);
      args->frame_buff[y * args->camera->image_width + x] = c;
    }
  }

  return 0;
}

void camera_render_multithread(Camera *camera, Hittable *world,
                               Color *frame_buff) {

  size_t size_of_thread_args = sizeof(ThreadArgs);
  int number_of_threads = multithreading_number_of_threads();
  HANDLE *threads = (HANDLE *)malloc(number_of_threads * sizeof(HANDLE));
  ThreadArgs *thread_args =
      (ThreadArgs *)malloc(number_of_threads * size_of_thread_args);

  LARGE_INTEGER freq, start, end;
  QueryPerformanceFrequency(&freq);
  QueryPerformanceCounter(&start);

  for (int i = 0; i < number_of_threads; i++) {
    thread_args[i] = (ThreadArgs){.thread_id = i,
                                  .number_of_threads = number_of_threads,
                                  .camera = camera,
                                  .world = world,
                                  .frame_buff = frame_buff};

    threads[i] =
        CreateThread(NULL, 0, _render_multithread, &thread_args[i], 0, NULL);

    if (threads[i] == NULL) {
      fprintf(stderr, "Failed to create thread %d/n", i);
    }
  }

  WaitForMultipleObjects(number_of_threads, threads, TRUE, INFINITE);

  QueryPerformanceCounter(&end);
  double elapsed = (double)(end.QuadPart - start.QuadPart) / freq.QuadPart;
  fprintf(stderr, "Render time: %.3f seconds \n", elapsed);

  for (int i = 0; i < number_of_threads; i++) {
    CloseHandle(threads[i]);
  }

  free(threads);
  free(thread_args);
}

#endif

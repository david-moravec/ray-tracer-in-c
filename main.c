#include "rtcommon.h"
#include "stdbool.h"
#include "stdint.h"
#include "time.h"

#include "base/include/arena.h"

#include "camera.c"
#include "hittable.c"
#include "material.c"

int main() {
  Arena *arena = arena_alloc(1 << 30, 0);
  Hittable world = hittable_collection_new(arena);

  Material material_ground = material_new_lambertian(color_new(0.5, 0.5, 0.5));
  hittable_list_add(world.collection,
                    hittable_sphere_new(point3_new(0.0, -1000.0, 0.0), 1000.0,
                                        &material_ground));

  for (int a = -11; a < 11; a++) {
    for (int b = -11; b < 11; b++) {
      double choose_mat = random_double();
      Point3 center =
          point3_new(a + 0.9 * random_double(), 0.2, b + 0.9 * random_double());

      if ((vec3_length(vec3_subtract(center, point3_new(4, 0.2, 0)))) > 0.9) {
        Material *sphere_material = arena_push(arena, sizeof(Material));

        if (choose_mat < 0.8) {
          Color albedo = vec3_multiply(vec3_random(), vec3_random());
          sphere_material->type = RAYTRACER_MATERIAL_LAMBERTIAN;
          sphere_material->albedo = albedo;
        } else if (choose_mat < 0.95) {
          Color albedo = vec3_random_in_interval(0.5, 1.0);
          double fuzz = random_double_in_interval(0.0, 0.5);
          sphere_material->type = RAYTRACER_MATERIAL_METAL;
          sphere_material->albedo = albedo;
          sphere_material->fuzz = fuzz;
        } else {
          sphere_material->type = RAYTRACER_MATERIAL_DIELECTRICS;
          sphere_material->refraction_index = 1.5;
        }

        hittable_list_add(world.collection,
                          hittable_sphere_new(center, 0.2, sphere_material));
      }
    }
  }

  Material material_1 = material_new_dielectrics(1.50);
  hittable_list_add(
      world.collection,
      hittable_sphere_new(point3_new(0.0, 1.0, 0), 1, &material_1));

  Material material_2 = material_new_lambertian(color_new(0.4, 0.2, 0.1));
  hittable_list_add(
      world.collection,
      hittable_sphere_new(point3_new(-4.0, 1, 0.0), 1.0, &material_2));

  Material material_3 = material_new_metal(color_new(0.7, 0.6, 0.5), 0.0);
  hittable_list_add(
      world.collection,
      hittable_sphere_new(point3_new(4.0, 1.0, 0.0), 1.0, &material_3));

  Camera camera = {0};
  camera.aspect_ratio = 16.0 / 9.0;
  camera.image_width = 400;
  camera.samples_per_pixel = 50;
  camera.max_depth = 50;

  camera.vfov = 20;
  camera.look_from = point3_new(13.0, 2, 3);
  camera.look_at = point3_new(0.0, 0.0, 0);
  camera.vup = vec3_new(0.0, 1.0, 0.0);

  camera.defocus_angle = 0.6;
  camera.focus_dist = 10.0;

  camera_initialize(&camera);

  double start_time = (double)clock() / CLOCKS_PER_SEC;
  uint32_t pixel_count = camera.image_height * camera.image_width;
  Color *frame_buff = (Color *)arena_push(arena, pixel_count * sizeof(Color));
  camera_render_multithread(&camera, &world, frame_buff);
  double end_time = (double)clock() / CLOCKS_PER_SEC;

  // print to ppm
  printf("P3\n%u %u\n255\n", camera.image_width, camera.image_height);
  for (int i = 0; i < pixel_count; i++) {
    color_fprint(stdout, frame_buff[i]);
  }

  fprintf(stderr, "Elapsed time: %f\n", end_time - start_time);
}

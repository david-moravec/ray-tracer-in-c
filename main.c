#include "stdbool.h"
#include "stdint.h"
#include "time.h"

#include "base/include/arena.h"

#include "camera.c"
#include "hittable.c"
#include "material.c"

int main() {
  Arena *arena = arena_alloc(1 << 20, 0);
  Hittable world = hittable_collection_new(arena);

  double R = cos(M_PI_4);

  // Material material_ground = material_new_lambertian(color_new(0.8, 0.8,
  // 0.0)); Material material_center = material_new_lambertian(color_new(0.1,
  // 0.2, 0.5)); Material material_left = material_new_dielectrics(1.50);
  // Material material_bubble = material_new_dielectrics(1.00 / 1.50);
  // Material material_right = material_new_metal(color_new(0.8, 0.6,
  // 0.2), 1.0);

  Material material_left = material_new_lambertian(color_new(0.0, 0.0, 1.0));
  Material material_right = material_new_lambertian(color_new(1.0, 0.0, 0.0));

  hittable_list_add(
      world.collection,
      hittable_sphere_new(point3_new(-R, 0, -1.0), R, &material_left));
  hittable_list_add(
      world.collection,
      hittable_sphere_new(point3_new(R, 0, -1.0), R, &material_right));

  Camera camera = {0};
  camera.aspect_ratio = 16.0 / 9.0;
  camera.image_width = 400;
  camera.samples_per_pixel = 100;
  camera.max_depth = 50;

  camera.vfov = 90;

  camera_initialize(&camera);
  double start_time = (double)clock() / CLOCKS_PER_SEC;
  camera_render(&camera, &world);
  double end_time = (double)clock() / CLOCKS_PER_SEC;

  fprintf(stderr, "Elapsed time: %f\n", end_time - start_time);
}

#include "material.h"
#include "stdbool.h"
#include "stdint.h"

#include "base/include/arena.h"

#include "camera.h"
#include "hittable.h"

int main() {
  Arena *arena = arena_alloc(1 << 20, 0);
  Hittable world = hittable_collection_new(arena);

  Material material_ground =
      material_new(RAYTRACER_MATERIAL_LAMBERTIAN, color_new(0.8, 0.8, 0.0));
  Material material_center =
      material_new(RAYTRACER_MATERIAL_LAMBERTIAN, color_new(0.1, 0.2, 0.5));
  Material material_left =
      material_new(RAYTRACER_MATERIAL_METAL, color_new(0.8, 0.8, 0.8));
  Material material_right =
      material_new(RAYTRACER_MATERIAL_METAL, color_new(0.8, 0.6, 0.2));

  hittable_list_add((HittableList *)world.collection,
                    hittable_sphere_new(point3_new(0.0, -100.5, -1.0), 100,
                                        &material_ground));
  hittable_list_add(
      (HittableList *)world.collection,
      hittable_sphere_new(point3_new(0.0, 0.0, -1.2), 0.5, &material_center));
  hittable_list_add(
      (HittableList *)world.collection,
      hittable_sphere_new(point3_new(-1.0, 0, -1.0), 0.5, &material_left));
  hittable_list_add(
      (HittableList *)world.collection,
      hittable_sphere_new(point3_new(1.0, 0, -1.0), 0.5, &material_right));

  Camera camera = {0};
  camera.aspect_ratio = 16.0 / 9.0;
  camera.image_width = 400;
  camera.samples_per_pixel = 100;
  camera.max_depth = 50;

  camera_initialize(&camera);
  camera_render(&camera, &world);
}

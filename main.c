#include "stdbool.h"
#include "stdint.h"

#include "base/include/arena.h"

#include "camera.h"
#include "hittable.h"

int main() {
  Arena *arena = arena_alloc(1 << 20, 0);
  Hittable world = hittable_collection_new(arena);

  hittable_list_add((HittableList *)world.collection,
                    hittable_sphere_new(point3_new(0.0, 0.0, -1.0), 0.5));

  hittable_list_add((HittableList *)world.collection,
                    hittable_sphere_new(point3_new(0.0, -100.5, -1.0), 100));

  Camera camera = {0};
  camera.aspect_ratio = 16.0 / 9.0;
  camera.image_width = 400;
  camera.samples_per_pixel = 100;

  camera_initialize(&camera);
  camera_render(&camera, &world);
}

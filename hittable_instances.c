#include "math.h"

#include "hittable.c"
#include "material.c"
#include "vec3.c"

Hittable hittable_instances_box_new(
    Point3 a, Point3 b, Material *material, Arena *arena
) {
  Hittable box = hittable_collection_new(arena);

  Point3 p_min = point3_new(fmin(a.x, b.x), fmin(a.y, b.y), fmin(a.z, b.z));
  Point3 p_max = point3_new(fmax(a.x, b.x), fmax(a.y, b.y), fmax(a.z, b.z));

  Vec3 dx = vec3_new(p_max.x - p_min.x, 0, 0);
  Vec3 dy = vec3_new(0, p_max.y - p_min.y, 0);
  Vec3 dz = vec3_new(0, 0, p_max.z - p_min.z);

  hittable_list_add(
      &box,
      hittable_quad_new(point3_new(p_min.x, p_min.y, p_max.z), dx, dy, material)
  );
  hittable_list_add(
      &box,
      hittable_quad_new(
          point3_new(p_max.x, p_min.y, p_max.z), vec3_negative(dz), dy, material
      )
  );
  hittable_list_add(
      &box,
      hittable_quad_new(
          point3_new(p_max.x, p_min.y, p_min.z), vec3_negative(dx), dy, material
      )
  );
  hittable_list_add(
      &box,
      hittable_quad_new(point3_new(p_min.x, p_min.y, p_min.z), dz, dy, material)
  );
  hittable_list_add(
      &box,
      hittable_quad_new(
          point3_new(p_min.x, p_max.y, p_max.z), dx, vec3_negative(dz), material
      )
  );
  hittable_list_add(
      &box,
      hittable_quad_new(point3_new(p_min.x, p_min.y, p_min.z), dx, dz, material)
  );

  return box;
}

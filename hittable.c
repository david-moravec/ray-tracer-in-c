#ifndef RAYTRACER_HITTABLE_C
#define RAYTRACER_HITTABLE_C

#include "assert.h"

#include "base/include/arena.h"
#include "base/include/list.h"

#include "aabb.c"
#include "interval.c"
#include "ray.c"
#include "vec3.c"

struct _Material;

typedef struct {
  Point3 p;
  Vec3 normal;
  double t;
  struct _Material *material;
  double u;
  double v;
  bool front_face;
} HitRecord;

typedef enum HittableTypeEnum {
  RAYTRACER_HITTABLE_SPHERE,
  RAYTRACER_HITTABLE_COLLECTION,
  RAYTRACER_HITTABLE_BHV_NODE,
} HittableType;

struct _Hittable;
typedef List(struct _Hittable) HittableList;

typedef struct _Hittable {
  HittableType type;
  union {
    // sphere
    struct {
      Point3 center;
      double r;
      struct _Material *material;
    };

    // list
    HittableList *collection;

    // bhv_node;
    struct {
      struct _Hittable *left;
      struct _Hittable *right;
    };
  };
  Aabb bounding_box;
} Hittable;

bool hittable_hit(const Hittable *hittable, Ray ray, Interval ray_t,
                  HitRecord *record);

void hittable_list_add(Hittable *list, Hittable hittable) {
  list_push(list->collection, hittable);
  list->bounding_box = aabb_union(list->bounding_box, hittable.bounding_box);
}

Hittable hittable_collection_new(Arena *arena) {
  HittableList *list = (HittableList *)arena_push(arena, sizeof(HittableList));
  list_init(list, arena);
  Hittable result = (Hittable){.type = RAYTRACER_HITTABLE_COLLECTION,
                               .collection = list,
                               .bounding_box = aabb_new_empty()};

  return result;
}

Hittable hittable_sphere_new(Point3 center, double radius,
                             struct _Material *material) {
  Vec3 rvec = vec3_new(radius, radius, radius);

  return (Hittable){.type = RAYTRACER_HITTABLE_SPHERE,
                    .center = center,
                    .r = radius,
                    .material = material,
                    .bounding_box = aabb_new(vec3_subtract(center, rvec),
                                             vec3_add(center, rvec))};
}

Hittable hittable_bhv_node_new(HittableList *hittables, size_t start,
                               size_t end, Arena *arena) {
  Hittable bhv_node = (Hittable){.type = RAYTRACER_HITTABLE_BHV_NODE};

  Aabb bbox = aabb_new_empty();

  for (size_t object_index = start; object_index < end; object_index++) {
    Hittable *object = list_get(hittables, object_index);
    bbox = aabb_union(bbox, object->bounding_box);
  }

  int axis = aabb_longest_axis(bbox);

  compare_function comparator = (axis == 0)   ? aabb_x_compare
                                : (axis == 1) ? aabb_y_compare
                                              : aabb_z_compare;

  size_t object_span = end - start;

  if (object_span == 1) {
    bhv_node.left = list_get(hittables, start);
    bhv_node.right = list_get(hittables, start);
  } else if (object_span == 2) {
    bhv_node.left = list_get(hittables, start);
    bhv_node.right = list_get(hittables, start + 1);
  } else {
    list_sort_in_interval(hittables, start, end, comparator);

    size_t mid = start + object_span / 2;
    Hittable left = hittable_bhv_node_new(hittables, start, mid, arena);
    bhv_node.left = arena_push_copy(arena, sizeof(Hittable), &left);
    Hittable right = hittable_bhv_node_new(hittables, mid, end, arena);
    bhv_node.right = arena_push_copy(arena, sizeof(Hittable), &right);
  }

  bhv_node.bounding_box = bbox;

  return bhv_node;
}

void hit_record_set_face_normal(HitRecord *record, const Ray ray,
                                const Vec3 outward_normal) {
#ifdef DEBUG
  assert(fabs(vec3_length_squared(outward_normal) - 1.0) < 1e-12);
#endif
  record->front_face = vec3_dot_product(ray.direction, outward_normal) < 0;
  record->normal =
      record->front_face ? outward_normal : vec3_negative(outward_normal);
}

void hittable_transform_to_sphere_coordinates(Point3 p, double *u, double *v) {
  double theta = acos(-p.y);
  double phi = atan2(-p.z, p.x) + M_PI;

  *u = phi / (2.0 * M_PI);
  *v = theta / M_PI;
}

bool hittable_sphere_hit(const Hittable *hittable, const Ray ray,
                         Interval ray_t, HitRecord *record) {
  assert(hittable->type == RAYTRACER_HITTABLE_SPHERE);

  Vec3 origin_to_center = vec3_subtract(hittable->center, ray.origin);

  double a = vec3_length_squared(ray.direction);
  double h = vec3_dot_product(ray.direction, origin_to_center);
  double c = vec3_length_squared(origin_to_center) - hittable->r * hittable->r;
  double discriminant = h * h - a * c;

  if (discriminant < 0) {
    return false;
  }

  double sqrt_discriminant = sqrt(discriminant);
  double root = (h - sqrt_discriminant) / a;

  if (!interval_sorrounds(ray_t, root)) {
    root = (h + sqrt_discriminant) / a;
    if (!interval_sorrounds(ray_t, root)) {
      return false;
    }
  }

  record->t = root;
  record->p = ray_at(ray, root);
  Vec3 outward_normal = vec3_scalar_devide(
      vec3_subtract(record->p, hittable->center), hittable->r);
  hit_record_set_face_normal(record, ray, outward_normal);
  hittable_transform_to_sphere_coordinates(outward_normal, &record->u,
                                           &record->v);
  record->material = hittable->material;

  return true;
}

bool hittable_list_hit(const Hittable *collection, Ray ray, Interval ray_t,
                       HitRecord *record) {
  HitRecord temp_record;
  bool anything_hitted = false;
  double current_closest = ray_t.max;

  HittableList *list = collection->collection;

  size_t index = 0;

  for (list_iterate(list, index)) {
    Hittable *hittable = list_get(list, index);

    if (hittable_hit(hittable, ray, interval_new(ray_t.min, current_closest),
                     &temp_record)) {
      anything_hitted = true;
      current_closest = temp_record.t;
      *record = temp_record;
    }
  }

  return anything_hitted;
}

bool hittable_bhv_node_hit(const Hittable *hittable, Ray ray, Interval ray_t,
                           HitRecord *record) {
  if (!aabb_hit(hittable->bounding_box, ray, ray_t)) {
    return false;
  }

  bool hit_left = hittable_hit(hittable->left, ray, ray_t, record);
  bool hit_right = hittable_hit(
      hittable->right, ray,
      interval_new(ray_t.min, hit_left ? record->t : ray_t.max), record);

  return hit_left || hit_right;
}

bool hittable_hit(const Hittable *hittable, Ray ray, Interval ray_t,
                  HitRecord *record) {
  switch (hittable->type) {
  case RAYTRACER_HITTABLE_SPHERE:
    return hittable_sphere_hit(hittable, ray, ray_t, record);
  case RAYTRACER_HITTABLE_COLLECTION:
    return hittable_list_hit(hittable, ray, ray_t, record);
  case RAYTRACER_HITTABLE_BHV_NODE:
    return hittable_bhv_node_hit(hittable, ray, ray_t, record);
  }
}

#endif

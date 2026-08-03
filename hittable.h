#ifndef RAYTRACER_HITTABLE_C
#define RAYTRACER_HITTABLE_C

#include "assert.h"
#include "base/include/arena.h"
#include "base/include/list.h"
// #include "hittable_list.h"
#include "ray.h"
#include "vec3.h"

typedef struct {
  Point3 p;
  Vec3 normal;
  double t;
  bool front_face;
} HitRecord;

typedef enum HittableTypeEnum {
  RAYTRACER_HITTABLE_SPHERE,
  RAYTRACER_HITTABLE_COLLECTION
} HittableType;

struct _Hittable;
typedef List(struct _Hittable) HittableList;

typedef struct _Hittable {
  HittableType type;
  union {
    struct {
      Point3 center;
      double r;
    };
    HittableList *collection;
  };
} Hittable;

bool hittable_hit(const Hittable *hittable, Ray ray, double ray_tmin,
                  double ray_tmax, HitRecord *record);

static inline void hittable_list_add(HittableList *list, Hittable hittable) {
  list_push(list, hittable);
}

static inline Hittable hittable_collection_new(Arena *arena) {
  HittableList *list = (HittableList *)arena_push(arena, sizeof(HittableList));
  list_init(list, arena);
  Hittable result =
      (Hittable){.type = RAYTRACER_HITTABLE_COLLECTION, .collection = list};

  return result;
}

static inline Hittable hittable_sphere_new(Point3 center, double radius) {
  return (Hittable){
      .type = RAYTRACER_HITTABLE_SPHERE, .center = center, .r = radius};
}

static inline void hit_record_set_face_normal(HitRecord *record, const Ray ray,
                                              const Vec3 outward_normal) {
#ifdef DEBUG
  assert(fabs(vec3_length_squared(outward_normal) - 1.0) < 1e-12);
#endif
  record->front_face = vec3_dot_product(ray.direction, outward_normal) < 0;
  record->normal =
      record->front_face ? outward_normal : vec3_negative(outward_normal);
}

static inline bool hittable_sphere_hit(const Hittable *hittable, const Ray ray,
                                       double ray_tmin, double ray_tmax,
                                       HitRecord *record) {
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

  if (root <= ray_tmin || ray_tmax <= root) {
    root = (h + sqrt_discriminant) / a;
    if (root <= ray_tmin || ray_tmax <= root) {
      return false;
    }
  }

  record->t = root;
  record->p = ray_at(ray, root);
  Vec3 outward_normal = vec3_scalar_devide(
      vec3_subtract(record->p, hittable->center), hittable->r);
  hit_record_set_face_normal(record, ray, outward_normal);

  return true;
}

static inline bool hittable_list_hit(HittableList *list, Ray ray,
                                     double ray_tmin, double ray_tmax,
                                     HitRecord *record) {
  HitRecord temp_record;
  bool anything_hitted = false;
  double current_closest = ray_tmax;

  size_t index = 0;

  for (list_iterate(*list, index)) {
    Hittable *hittable = list_get(*list, index);

    if (hittable_hit(hittable, ray, ray_tmin, ray_tmax, record)) {
      anything_hitted = true;
      current_closest = temp_record.t;
      *record = temp_record;
    }
  }

  return anything_hitted;
}

inline bool hittable_hit(const Hittable *hittable, Ray ray, double ray_tmin,
                         double ray_tmax, HitRecord *record) {
  switch (hittable->type) {
  case RAYTRACER_HITTABLE_SPHERE:
    return hittable_sphere_hit(hittable, ray, ray_tmin, ray_tmax, record);
  case RAYTRACER_HITTABLE_COLLECTION:
    return hittable_list_hit(hittable->collection, ray, ray_tmin, ray_tmax,
                             record);
  }
}

#endif

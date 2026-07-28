#ifndef RAYTRACER_HITTABLE_C
#define RAYTRACER_HITTABLE_C

#include "assert.h"
#include "ray.h"
#include "vec3.h"

typedef struct {
  Point3 p;
  Vec3 normal;
  double t;
} HitRecord;

enum HittableType { RAYTRACER_HITTABLE_SPHERE };

typedef struct {
  HittableType type;
  union {
    struct {
      Point3 center;
      double r;
    };
  };
} Hittable;

static inline Hittable hittable_sphere_new(Point3 center, double radius) {
  return (Hittable){
      .type = RAYTRACER_HITTABLE_SPHERE, .center = center, .r = radius};
}

static inline bool hittable_sphere_hit(Hittable hittable, const Ray ray,
                                       double ray_tmin, double ray_tmax,
                                       HitRecord *record) {
  assert(hittable.type == RAYTRACER_HITTABLE_SPHERE);

  Vec3 origin_to_center = vec3_subtract(hittable.center, ray.origin);

  double a = vec3_length_squared(ray.direction);
  double h = vec3_dot_product(ray.direction, origin_to_center);
  double c = vec3_length_squared(origin_to_center) - hittable.r * hittable.r;
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
  record->normal =
      vec3_scalar_devide(vec3_subtract(record->p, hittable.center), hittable.r);

  return true;
}

inline bool hittable_hit(Hittable hittable, Ray ray, double ray_tmin,
                         double ray_tmax, HitRecord *record) {
  switch (hittable.type) {
  case RAYTRACER_HITTABLE_SPHERE:
    return hittable_sphere_hit(hittable, ray, ray_tmin, ray_tmax, record);
  }
}

#endif

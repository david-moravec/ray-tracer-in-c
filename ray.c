#ifndef RAYTRACER_RAY_C
#define RAYTRACER_RAY_C

#include "vec3.c"

typedef struct {
  Vec3 origin;
  Point3 direction;
} Ray;

static inline Ray ray_new(Vec3 origin, Point3 direction) {
  return (Ray){.origin = origin, .direction = direction};
}

static inline Point3 ray_at(const Ray ray, double t) {
  return vec3_add(ray.origin, vec3_scalar_multiply(ray.direction, t));
}

#endif

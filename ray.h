#ifndef RAYTRACER_RAY_H
#define RAYTRACER_RAY_H

#include "vec3.h"

typedef struct {
  Vec3 origin;
  Point3 direction;
} Ray;

static inline Point3 ray_at(const Ray ray, double t) {
  return vec3_add(ray.origin, vec3_scalar_multiply(ray.direction, t));
}

#endif

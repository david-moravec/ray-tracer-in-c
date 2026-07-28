#ifndef RAYTRACER_VEC3_H
#define RAYTRACER_VEC3_H

#include "math.h"
#include "stdint.h"
#include "stdio.h"

typedef struct {
  union {
    double e[3];
    struct {
      double x;
      double y;
      double z;
    };
  };

} Vec3;

static inline Vec3 vec3_add(const Vec3 v1, const Vec3 v2) {
  Vec3 result;

  result.x = v1.x + v2.x;
  result.y = v1.y + v2.y;
  result.z = v1.z + v2.z;

  return result;
}

static inline Vec3 vec3_subtract(const Vec3 v1, const Vec3 v2) {
  Vec3 result;

  result.x = v1.x - v2.x;
  result.y = v1.y - v2.y;
  result.z = v1.z - v2.z;

  return result;
}

static inline Vec3 vec3_multiply(const Vec3 v1, const Vec3 v2) {
  Vec3 result;

  result.x = v1.x * v2.x;
  result.y = v1.y * v2.y;
  result.z = v1.z * v2.z;

  return result;
}

static inline Vec3 vec3_devide(const Vec3 v1, const Vec3 v2) {
  Vec3 result;

  result.x = v1.x / v2.x;
  result.y = v1.y / v2.y;
  result.z = v1.z / v2.z;

  return result;
}

static inline Vec3 vec3_negative(const Vec3 v1) {
  Vec3 result;
  result.x = -v1.x;
  result.y = -v1.y;
  result.z = -v1.z;

  return result;
}

static inline void vec3_add_mut(Vec3 *v1, const Vec3 v2) {
  v1->x += v2.x;
  v1->y += v2.y;
  v1->z += v2.z;
}

static inline void vec3_subtract_mut(Vec3 *v1, const Vec3 v2) {
  v1->x -= v2.x;
  v1->y -= v2.y;
  v1->z -= v2.z;
}

static inline void vec3_multiply_mut(Vec3 *v1, const Vec3 v2) {
  v1->x *= v2.x;
  v1->y *= v2.y;
  v1->z *= v2.z;
}

static inline void vec3_devide_mut(Vec3 *v1, const Vec3 v2) {
  v1->x /= v2.x;
  v1->y /= v2.y;
  v1->z /= v2.z;
}

static inline double vec3_length_squared(const Vec3 v) {
  return v.x * v.x + v.y * v.y + v.z * v.z;
}

static inline double vec3_length(const Vec3 v) {
  return sqrt(vec3_length_squared(v));
}

typedef Vec3 Point3;

static inline void vec3_printf(FILE *f, const Vec3 v) {
  fprintf(f, "(%.6f, %.6f, %.6f)", v.e[0], v.e[1], v.e[2]);
}

static inline Vec3 vec3_scalar_add(const Vec3 v, const double t) {
  Vec3 result;
  result.x = v.x + t;
  result.y = v.y + t;
  result.z = v.z + t;

  return result;
}

static inline Vec3 vec3_scalar_subtract(const Vec3 v, const double t) {
  Vec3 result;
  result.x = v.x - t;
  result.y = v.y - t;
  result.z = v.z - t;

  return result;
}

static inline Vec3 vec3_scalar_devide(const Vec3 v, const double t) {
  Vec3 result;
  result.x = v.x / t;
  result.y = v.y / t;
  result.z = v.z / t;

  return result;
}
static inline Vec3 vec3_scalar_multiply(const Vec3 v, const double t) {
  Vec3 result;
  result.x = v.x * t;
  result.y = v.y * t;
  result.z = v.z * t;

  return result;
}

static inline double vec3_dot_product(const Vec3 v1, const Vec3 v2) {
  return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

static inline Vec3 vec3_cross_product(const Vec3 v1, const Vec3 v2) {
  Vec3 result;
  result.x = v1.y * v2.z - v1.z * v2.y;
  result.y = v1.z * v2.x - v1.x * v2.z;
  result.z = v1.x * v2.y - v1.y * v2.x;
  return result;
}

static inline Vec3 vec3_unit_vector(const Vec3 v) {
  return vec3_scalar_devide(v, vec3_length(v));
}

static inline Vec3 vec3_new(double x, double y, double z) {
  return (Vec3){.x = x, .y = y, .z = z};
}

static inline Point3 point3_new(double x, double y, double z) {
  return vec3_new(x, y, z);
}

#endif

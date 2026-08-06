#ifndef RAYTRACER_VEC3_C
#define RAYTRACER_VEC3_C

#include "math.h"
#include "stdbool.h"

#include "rtcommon.h"
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

static inline Vec3 vec3_random() {
  return vec3_new(random_double(), random_double(), random_double());
}

static inline Vec3 vec3_random_in_interval(double min, double max) {
  return vec3_new(random_double_in_interval(min, max),
                  random_double_in_interval(min, max),
                  random_double_in_interval(min, max));
}

static inline Vec3 random_unit_vector() {
  while (true) {
    Vec3 random = vec3_random_in_interval(-1, 1);
    double lensq = vec3_length_squared(random);
    if (1e-160 < lensq && lensq <= 1) {
      return vec3_scalar_devide(random, sqrt(lensq));
    }
  }
}

static inline Vec3 vec3_random_on_hemishpere(Vec3 normal) {
  Vec3 on_unit_sphere = random_unit_vector();
  if (vec3_dot_product(on_unit_sphere, normal) > 0.0) {
    return on_unit_sphere;
  } else {
    return vec3_negative(on_unit_sphere);
  }
}

static inline bool vec3_near_zero(Vec3 v) {
  double epsilon = 1e-8;
  return (fabs(v.x) < epsilon) && (fabs(v.y) < epsilon) &&
         (fabs(v.z) < epsilon);
}

static inline Vec3 vec3_reflect(Vec3 v, Vec3 n) {
  return vec3_subtract(v, vec3_scalar_multiply(n, 2 * vec3_dot_product(v, n)));
}

static inline Vec3 vec3_refract(Vec3 uv, Vec3 n,
                                double refraction_index_ratio) {
  double cos_theta = fmin(vec3_dot_product(vec3_negative(uv), n), 1.0);
  Vec3 r_out_perp = vec3_scalar_multiply(
      vec3_add(uv, vec3_scalar_multiply(n, cos_theta)), refraction_index_ratio);
  Vec3 r_out_parallel = vec3_scalar_multiply(
      n, -sqrt(fabs(1.0 - vec3_length_squared(r_out_perp))));

  return vec3_add(r_out_perp, r_out_parallel);
}

#endif

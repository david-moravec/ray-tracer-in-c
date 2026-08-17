#ifndef RAYTRACER_PERLIN_C
#define RAYTRACER_PERLIN_C

#include "rtcommon.h"
#include "vec3.c"
#include <math.h>
enum { _DEFAULT_POINT_COUNT = 256 };

typedef struct {
  int point_count;
  Vec3 randvec[_DEFAULT_POINT_COUNT];
  int perm_x[_DEFAULT_POINT_COUNT];
  int perm_y[_DEFAULT_POINT_COUNT];
  int perm_z[_DEFAULT_POINT_COUNT];

} Perlin;

static void permute(int *p, int n) {
  for (int i = n - 1; i > 0; i--) {
    int target = random_int_in_interval(0, i);
    int tmp = p[i];
    p[i] = p[target];
    p[target] = tmp;
  }
}

static void generate_perm(int *p, int point_count) {
  for (int i = 0; i < point_count; i++) {
    p[i] = i;
  }

  permute(p, point_count);
}

static double perlin_interp(const Vec3 c[2][2][2], double u, double v,
                            double w) {
  double uu = u * u * (3 - 2 * u);
  double vv = v * v * (3 - 2 * v);
  double ww = w * w * (3 - 2 * w);

  double result = 0.0;

  for (int i = 0; i < 2; i++) {
    for (int j = 0; j < 2; j++) {
      for (int k = 0; k < 2; k++) {
        Vec3 weigth_v = vec3_new(u - i, v - j, w - k);
        result += (i * uu + (1 - i) * (1 - uu)) *
                  (j * vv + (1 - j) * (1 - vv)) *
                  (k * ww + (1 - k) * (1 - ww)) *
                  vec3_dot_product(c[i][j][k], weigth_v);
      }
    }
  }
  return result;
}

double perlin_noise(Perlin *perlin, Point3 p) {
  double u = p.x - floor(p.x);
  double v = p.y - floor(p.y);
  double w = p.z - floor(p.z);

  int i = (int)(floor(p.x));
  int j = (int)(floor(p.y));
  int k = (int)(floor(p.z));
  Vec3 c[2][2][2];

  for (int di = 0; di < 2; di++) {
    for (int dj = 0; dj < 2; dj++) {
      for (int dk = 0; dk < 2; dk++) {
        c[di][dj][dk] = perlin->randvec[perlin->perm_x[(i + di) & 255] ^
                                        perlin->perm_y[(j + dj) & 255] ^
                                        perlin->perm_z[(k + dk) & 255]];
      }
    }
  }

  return perlin_interp(c, u, v, w);
}

double perlin_turbulence(Perlin *perlin, Point3 p, int depth) {
  double result = 0.0;
  Point3 temp_p = p;
  double weigth = 1.0;

  for (int i = 0; i < depth; i++) {
    result += weigth * perlin_noise(perlin, temp_p);
    weigth *= 0.5;
    temp_p = vec3_scalar_multiply(temp_p, 2.0);
  }

  return fabs(result);
}

Perlin perlin_new() {
  Perlin result = {0};
  result.point_count = _DEFAULT_POINT_COUNT;

  for (int i = 0; i < result.point_count; i++) {
    result.randvec[i] = random_unit_vector();
  }

  generate_perm(result.perm_x, result.point_count);
  generate_perm(result.perm_y, result.point_count);
  generate_perm(result.perm_z, result.point_count);

  return result;
}

#endif

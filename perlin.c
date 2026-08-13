#ifndef RAYTRACER_PERLIN_C
#define RAYTRACER_PERLIN_C

#include "rtcommon.h"
#include "vec3.c"
#include <math.h>
enum { _DEFAULT_POINT_COUNT = 256 };

typedef struct {
  int point_count;
  double random_doubles[_DEFAULT_POINT_COUNT];
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

static double trilinear_interp(double c[2][2][2], double u, double v,
                               double w) {
  double result = 0.0;

  for (int i = 0; i < 2; i++) {
    for (int j = 0; j < 2; j++) {
      for (int k = 0; k < 2; k++) {
        result += ((i * u) + (1 - i) * (1 - u)) * (j * v + (1 - j) * (1 - v)) *
                  (k * w + (1 - k) * (1 - w)) * c[i][j][k];
      }
    }
  }
  return result;
}

double perlin_noise(Perlin *perlin, Point3 p) {
  double u = p.x - floor(p.x);
  double v = p.y - floor(p.y);
  double w = p.z - floor(p.z);

  u = u * u * (3 - 2 * u);
  v = v * v * (3 - 2 * v);
  w = w * w * (3 - 2 * w);

  int i = (int)(floor(p.x));
  int j = (int)(floor(p.y));
  int k = (int)(floor(p.z));
  double c[2][2][2];

  for (int di = 0; di < 2; di++) {
    for (int dj = 0; dj < 2; dj++) {
      for (int dk = 0; dk < 2; dk++) {
        c[di][dj][dk] = perlin->random_doubles[perlin->perm_x[(i + di) & 255] ^
                                               perlin->perm_y[(j + dj) & 255] ^
                                               perlin->perm_z[(k + dk) & 255]];
      }
    }
  }

  return trilinear_interp(c, u, v, w);
}

Perlin perlin_new() {
  Perlin result = {0};
  result.point_count = _DEFAULT_POINT_COUNT;

  for (int i = 0; i < result.point_count; i++) {
    result.random_doubles[i] = random_double();
  }

  generate_perm(result.perm_x, result.point_count);
  generate_perm(result.perm_y, result.point_count);
  generate_perm(result.perm_z, result.point_count);

  return result;
}

#endif

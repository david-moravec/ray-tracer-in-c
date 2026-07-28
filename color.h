#ifndef RAYTRACER_COLOR_H
#define RAYTRACER_COLOR_H

#include "stdio.h"
#include "vec3.h"

typedef Vec3 Color;

static inline Color color_new(double x, double y, double z) {
  return vec3_new(x, y, z);
}

static inline void color_fprint(FILE *f, const Color *color) {
  double r = color->x;
  double g = color->y;
  double b = color->z;

  int rbyte = (int)(255.999 * r);
  int gbyte = (int)(255.999 * g);
  int bbyte = (int)(255.999 * b);

  fprintf(f, "%d %d %d\n", rbyte, gbyte, bbyte);
}

#endif

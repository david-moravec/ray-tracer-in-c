#ifndef RAYTRACER_COLOR_H
#define RAYTRACER_COLOR_H

#include "interval.h"
#include "stdio.h"
#include "vec3.h"

static const Interval _INTENSITY = (Interval){.min = 0.0, .max = 0.999};

typedef Vec3 Color;

static inline Color color_new(double x, double y, double z) {
  return vec3_new(x, y, z);
}

static inline double linear_to_gamma(double linear_component) {
  if (linear_component > 0) {
    return sqrt(linear_component);
  }

  return 0.0;
}

static inline void color_fprint(FILE *f, const Color color) {
  double r = color.x;
  double g = color.y;
  double b = color.z;

  r = linear_to_gamma(r);
  g = linear_to_gamma(g);
  b = linear_to_gamma(b);

  int rbyte = (int)(256 * interval_clamp_value(_INTENSITY, r));
  int gbyte = (int)(256 * interval_clamp_value(_INTENSITY, g));
  int bbyte = (int)(256 * interval_clamp_value(_INTENSITY, b));

  fprintf(f, "%d %d %d\n", rbyte, gbyte, bbyte);
}

#endif

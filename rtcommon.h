#ifndef RAYTRACER_RT_COMMON_H
#define RAYTRACER_RT_COMMON_H

#include "math.h"
#include "stdlib.h"

static inline double deg_to_rad(double degrees) {
  return degrees * M_PI / 180.0;
}

static inline double random_double() {
  return (double)rand() / ((double)RAND_MAX + 1.0);
}

static inline double random_double_in_interval(double min, double max) {
  return min + (max - min) * random_double();
}

#endif

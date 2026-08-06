#ifndef INTERVAL_H
#define INTERVAL_H

#include "math.h"
#include "stdbool.h"

typedef struct {
  double min;
  double max;
} Interval;

static inline Interval interval_new_empty() {
  return (Interval){.min = INFINITY, .max = -INFINITY};
}

static inline Interval interval_new(double min, double max) {
  return (Interval){.min = min, .max = max};
}

static inline double interval_size(Interval interval) {
  return interval.max - interval.min;
}

static inline bool interval_contains(Interval interval, double x) {
  return interval.min <= x && x <= interval.max;
}

static inline bool interval_sorrounds(Interval interval, double x) {
  return interval.min < x && x < interval.max;
}

static inline double interval_clamp_value(Interval interval, double x) {
  if (x < interval.min) {
    return interval.min;
  }
  if (x > interval.max) {
    return interval.max;
  }

  return x;
}

static Interval INTERVAL_EMPTY = (Interval){.min = INFINITY, .max = -INFINITY};
static Interval INTERVAL_UNIVERSE =
    (Interval){.min = -INFINITY, .max = INFINITY};

#endif

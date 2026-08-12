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

static inline Interval interval_expand(Interval interval, double delta) {
  double padding = delta / 2.0;

  return interval_new(interval.min - padding, interval.max + padding);
}

static inline Interval interval_new_enclose(Interval i0, Interval i1) {
  return interval_new(fmin(i0.min, i1.min), fmax(i0.max, i1.max));
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

static inline double clamp_value(double min, double max, double x) {
  if (x < min) {
    return min;
  }
  if (x > max) {
    return max;
  }

  return x;
}

static inline double interval_clamp_value(Interval interval, double x) {
  return clamp_value(interval.min, interval.max, x);
}

static Interval INTERVAL_EMPTY = (Interval){.min = INFINITY, .max = -INFINITY};
static Interval INTERVAL_UNIVERSE =
    (Interval){.min = -INFINITY, .max = INFINITY};

#endif

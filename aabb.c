#ifndef AABB_C
#define AABB_C

#include "interval.c"
#include "ray.c"
#include "vec3.c"

typedef struct {
  union {
    Interval axis_interval[3];
    struct {
      Interval x, y, z;
    };
  };
} Aabb;

Aabb aabb_new_empty() {
  return (Aabb){.x = interval_new_empty(),
                .y = interval_new_empty(),
                .z = interval_new_empty()};
}

Aabb aabb_new(Point3 a, Point3 b) {
  Interval x = a.x <= b.x ? interval_new(a.x, b.x) : interval_new(b.x, a.x);
  Interval y = a.y <= b.y ? interval_new(a.y, b.y) : interval_new(b.y, a.y);
  Interval z = a.z <= b.z ? interval_new(a.z, b.z) : interval_new(b.z, a.z);

  return (Aabb){.x = interval_pad_to_minimum(x),
                .y = interval_pad_to_minimum(y),
                .z = interval_pad_to_minimum(z)};
}

Aabb aabb_union(Aabb box0, Aabb box1) {
  Interval x = interval_new_enclose(box0.x, box1.x);
  Interval y = interval_new_enclose(box0.y, box1.y);
  Interval z = interval_new_enclose(box0.z, box1.z);

  return (Aabb){.x = x, .y = y, .z = z};
}

Aabb aabb_from_diagonals(Point3 q, Vec3 diag0, Vec3 diag1) {
  Aabb bbox_diag_0 = aabb_new(q, vec3_add(q, vec3_add(diag0, diag1)));
  Aabb bbox_diag_1 = aabb_new(vec3_add(q, diag0), vec3_add(q, diag1));

  return aabb_union(bbox_diag_0, bbox_diag_1);
}

bool aabb_hit(Aabb box, Ray ray, Interval ray_t) {
  for (int axis = 0; axis < 3; axis++) {
    Interval ax = box.axis_interval[axis];
    double adinv = 1.0 / ray.direction.e[axis];

    double t0 = (ax.min - ray.origin.e[axis]) * adinv;
    double t1 = (ax.max - ray.origin.e[axis]) * adinv;

    if (t0 < t1) {
      if (t0 > ray_t.min) {
        ray_t.min = t0;
      }
      if (t1 < ray_t.max) {
        ray_t.max = t1;
      }
    } else {
      if (t1 > ray_t.min) {
        ray_t.min = t1;
      }
      if (t0 < ray_t.max) {
        ray_t.max = t0;
      }
    }

    if (ray_t.max <= ray_t.min) {
      return false;
    }
  }

  return true;
}

int aabb_compare(const Aabb *box0, const Aabb *box1, int axis_index) {
  return box0->axis_interval[axis_index].min <
         box1->axis_interval[axis_index].min;
}

int aabb_x_compare(const void *box0, const void *box1) {
  return aabb_compare(box0, box1, 0);
}

int aabb_y_compare(const void *box0, const void *box1) {
  return aabb_compare(box0, box1, 1);
}

int aabb_z_compare(const void *box0, const void *box1) {
  return aabb_compare(box0, box1, 2);
}

int aabb_longest_axis(Aabb bbox) {
  if (interval_size(bbox.x) > interval_size(bbox.y)) {
    return interval_size(bbox.x) > interval_size(bbox.z) ? 0 : 2;
  } else {
    return interval_size(bbox.y) > interval_size(bbox.z) ? 1 : 2;
  }
}

#endif

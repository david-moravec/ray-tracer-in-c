#ifndef RAYTRACER_HITTABLE_LIST_H
#define RAYTRACER_HITTABLE_LIST_H

#include "base/include/arena.h"
#include "base/include/list.h"
#include "hittable.h"
#include "stdbool.h"

typedef List(Hittable) HittableList;

inline void hittable_list_add(HittableList *list, Hittable hittable) {
  list_push(list, hittable);
}

inline bool hittable_list_hit(HittableList list, Ray ray, double ray_tmin,
                              double ray_tmax, HitRecord *record) {
  HitRecord temp_record;
  bool anything_hitted = false;
  double current_closest = ray_tmax;

  size_t index = 0;

  for (list_iterate(list, index)) {
    Hittable *hittable = list_get(list, index);

    if (hittable_hit(hittable, ray, ray_tmin, ray_tmax, record)) {
      anything_hitted = true;
      current_closest = temp_record.t;
      *record = temp_record;
    }
  }

  return anything_hitted;
}

#endif

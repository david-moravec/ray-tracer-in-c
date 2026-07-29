#ifndef RAYTRACER_HITTABLE_LIST_H
#define RAYTRACER_HITTABLE_LIST_H

#include "base/include/list.h"
#include "hittable.h"

typedef List(Hittable *) HittableList;

typedef struct {
  HittableList hittables;
} HittableCollection;

inline void hittable_collection_add(HittableCollection *collection,
                                    Hittable *hittable) {
  list_push(collection->hittables, hittable);
}

inline bool hittable_collection_hit(HittableCollection *collection, Ray ray,
                                    double ray_tmin, double ray_tmax,
                                    HitRecord *record) {
  HitRecord temp_record;
  bool anything_hitted = false;
  double current_closest = ray_tmax;

  size_t index = 0;

  for (list_iterate(collection->hittables, index)) {
    Hittable *hittable = list_get(collection->hittables, index);

    if (hittable_hit(*hittable, ray, ray_tmin, ray_tmax, record)) {
      anything_hitted = true;
      current_closest = temp_record.t;
      *record = temp_record;
    }
  }

  return anything_hitted;
}

#endif

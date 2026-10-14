#ifndef RAYTRACER_HITTABLE_C
#define RAYTRACER_HITTABLE_C

#include "assert.h"

#include "base/include/arena.h"
#include "base/include/list.h"

#include "aabb.c"
#include "interval.c"
#include "ray.c"
#include "rtcommon.h"
#include "vec3.c"
#include <math.h>

struct _Material;

typedef struct {
  Point3 p;
  Vec3 normal;
  double t;
  struct _Material *material;
  double u;
  double v;
  bool front_face;
} HitRecord;

typedef enum HittableTypeEnum {
  RAYTRACER_HITTABLE_SPHERE,
  RAYTRACER_HITTABLE_QUAD,
  RAYTRACER_HITTABLE_TRIANGLE,
  RAYTRACER_HITTABLE_COLLECTION,
  RAYTRACER_HITTABLE_BHV_NODE,
  RAYTRACER_HITTABLE_TRANSLATE,
  RAYTRACER_HITTABLE_ROTATE_Y,
  RAYTRACER_HITTABLE_CONSTANT_MEDIUM,
} HittableType;

struct _Hittable;
typedef List(struct _Hittable) HittableList;

typedef struct {
  Point3 center;
  double r;
} Sphere;

typedef struct {
  Point3 q;
  Vec3 u, v;
  Vec3 w;
  Vec3 normal;
  double d;
} Quad;

typedef struct {
  union {
    Point3 p[3];
    struct {
      Point3 a;
      Point3 b;
      Point3 c;
    };
  };
  double D;
  Vec3 u, v;
  Vec3 w;
  Vec3 normal;
} Triangle;

typedef struct {
  struct _Hittable *left;
  struct _Hittable *right;
} BhvNode;

typedef struct {
  Vec3 offset;
} Translate;

typedef struct {
  double sin_theta;
  double cos_theta;
} Rotate_Y;

typedef struct {
  struct _Hittable *object;

  union {
    Rotate_Y rotate_y;
    Translate translate;
  };
} Transformation;

typedef struct {
  const struct _Hittable *boundary;
  double neg_inv_density;
  struct _Material *phase_function;
} ConstantMedium;

typedef struct _Hittable {
  HittableType type;
  Aabb bounding_box;
  struct _Material *material;

  union {
    Sphere sphere;
    Quad quad;
    Triangle triangle;
    Transformation transormation;
    HittableList *collection;
    BhvNode bhv_node;
    ConstantMedium constant_medium;
  } as;
} Hittable;

bool hittable_hit(
    const Hittable *hittable, Ray ray, Interval ray_t, HitRecord *record
);

void hittable_list_add(Hittable *list, Hittable hittable) {
  list_push(list->as.collection, hittable);
  list->bounding_box = aabb_union(list->bounding_box, hittable.bounding_box);
}

Hittable hittable_collection_new(Arena *arena) {
  HittableList *list = (HittableList *)arena_push(arena, sizeof(HittableList));
  list_init(list, arena);
  Hittable result = (Hittable){
      .type = RAYTRACER_HITTABLE_COLLECTION,
      .as.collection = list,
      .bounding_box = aabb_new_empty()
  };

  return result;
}

Hittable
hittable_sphere_new(Point3 center, double radius, struct _Material *material) {
  Vec3 rvec = vec3_new(radius, radius, radius);

  return (Hittable){
      .type = RAYTRACER_HITTABLE_SPHERE,
      .as.sphere.center = center,
      .as.sphere.r = radius,
      .material = material,
      .bounding_box =
          aabb_new(vec3_subtract(center, rvec), vec3_add(center, rvec))
  };
}

Hittable hittable_bhv_node_new(
    HittableList *hittables, size_t start, size_t end, Arena *arena
) {
  BhvNode bhv_node;

  Aabb bbox = aabb_new_empty();

  for (size_t object_index = start; object_index < end; object_index++) {
    Hittable *object = list_get(hittables, object_index);
    bbox = aabb_union(bbox, object->bounding_box);
  }

  int axis = aabb_longest_axis(bbox);

  compare_function comparator = (axis == 0)   ? aabb_x_compare
                                : (axis == 1) ? aabb_y_compare
                                              : aabb_z_compare;

  size_t object_span = end - start;

  if (object_span == 1) {
    bhv_node.left = list_get(hittables, start);
    bhv_node.right = list_get(hittables, start);
  } else if (object_span == 2) {
    bhv_node.left = list_get(hittables, start);
    bhv_node.right = list_get(hittables, start + 1);
  } else {
    list_sort_in_interval(hittables, start, end, comparator);

    size_t mid = start + object_span / 2;
    Hittable left = hittable_bhv_node_new(hittables, start, mid, arena);
    bhv_node.left = arena_push_copy(arena, sizeof(Hittable), &left);
    Hittable right = hittable_bhv_node_new(hittables, mid, end, arena);
    bhv_node.right = arena_push_copy(arena, sizeof(Hittable), &right);
  }

  return (Hittable){
      .type = RAYTRACER_HITTABLE_BHV_NODE,
      .bounding_box = bbox,
      .as.bhv_node = bhv_node
  };
}

void hit_record_set_face_normal(
    HitRecord *record, const Ray ray, const Vec3 outward_normal
) {
#ifdef DEBUG
  assert(fabs(vec3_length_squared(outward_normal) - 1.0) < 1e-12);
#endif
  record->front_face = vec3_dot_product(ray.direction, outward_normal) < 0;
  record->normal =
      record->front_face ? outward_normal : vec3_negative(outward_normal);
}

void hittable_transform_to_sphere_coordinates(Point3 p, double *u, double *v) {
  double theta = acos(-p.y);
  double phi = atan2(-p.z, p.x) + M_PI;

  *u = phi / (2.0 * M_PI);
  *v = theta / M_PI;
}

bool hittable_hit_sphere(
    const Hittable *hittable, const Ray ray, Interval ray_t, HitRecord *record
) {

#ifdef DEBUG
  assert(hittable->type == RAYTRACER_HITTABLE_SPHERE);
#endif

  Sphere sphere = hittable->as.sphere;

  Vec3 origin_to_center = vec3_subtract(sphere.center, ray.origin);

  double a = vec3_length_squared(ray.direction);
  double h = vec3_dot_product(ray.direction, origin_to_center);
  double c = vec3_length_squared(origin_to_center) - sphere.r * sphere.r;
  double discriminant = h * h - a * c;

  if (discriminant < 0) {
    return false;
  }

  double sqrt_discriminant = sqrt(discriminant);
  double root = (h - sqrt_discriminant) / a;

  if (!interval_sorrounds(ray_t, root)) {
    root = (h + sqrt_discriminant) / a;
    if (!interval_sorrounds(ray_t, root)) {
      return false;
    }
  }

  record->t = root;
  record->p = ray_at(ray, root);
  Vec3 outward_normal =
      vec3_scalar_devide(vec3_subtract(record->p, sphere.center), sphere.r);
  hit_record_set_face_normal(record, ray, outward_normal);
  hittable_transform_to_sphere_coordinates(
      outward_normal, &record->u, &record->v
  );
  record->material = hittable->material;

  return true;
}

bool hittable_hit_list(
    const Hittable *hittable, Ray ray, Interval ray_t, HitRecord *record
) {
#ifdef DEBUG
  assert(hittable->type == RAYTRACER_HITTABLE_LIST);
#endif
  HitRecord temp_record;
  bool anything_hitted = false;
  double current_closest = ray_t.max;

  HittableList *list = hittable->as.collection;

  size_t index = 0;

  for (list_iterate(list, index)) {
    Hittable *hittable = list_get(list, index);

    if (hittable_hit(
            hittable,
            ray,
            interval_new(ray_t.min, current_closest),
            &temp_record
        )) {
      anything_hitted = true;
      current_closest = temp_record.t;
      *record = temp_record;
    }
  }

  return anything_hitted;
}

bool hittable_hit_bhv_node(
    const Hittable *hittable, Ray ray, Interval ray_t, HitRecord *record
) {
  if (!aabb_hit(hittable->bounding_box, ray, ray_t)) {
    return false;
  }

  bool hit_left = hittable_hit(hittable->as.bhv_node.left, ray, ray_t, record);
  bool hit_right = hittable_hit(
      hittable->as.bhv_node.right,
      ray,
      interval_new(ray_t.min, hit_left ? record->t : ray_t.max),
      record
  );

  return hit_left || hit_right;
}

// quad

Hittable
hittable_quad_new(Point3 q, Vec3 u, Vec3 v, struct _Material *material) {
  Vec3 n = vec3_cross_product(u, v);
  Vec3 normal = vec3_unit_vector(n);

  return (Hittable){
      .type = RAYTRACER_HITTABLE_QUAD,
      .material = material,
      .bounding_box = aabb_from_diagonals(q, u, v),
      .as.quad = (Quad){
          .q = q,
          .u = u,
          .v = v,
          .w = vec3_scalar_devide(n, vec3_dot_product(n, n)),
          .normal = normal,
          .d = vec3_dot_product(normal, q),

      }
  };
}

bool hittable_hit_quad(
    const Hittable *hittable, Ray ray, Interval ray_t, HitRecord *record
) {
  double denom = vec3_dot_product(hittable->as.quad.normal, ray.direction);

  if (fabs(denom) < 1e-8) {
    return false;
  }

  double t = (hittable->as.quad.d -
              vec3_dot_product(hittable->as.quad.normal, ray.origin)) /
             denom;

  if (!interval_contains(ray_t, t)) {
    return false;
  }

  Point3 intersection = ray_at(ray, t);
  Vec3 planar_hitpt_vector = vec3_subtract(intersection, hittable->as.quad.q);
  double alpha = vec3_dot_product(
      hittable->as.quad.w,
      vec3_cross_product(planar_hitpt_vector, hittable->as.quad.v)
  );
  double beta = vec3_dot_product(
      hittable->as.quad.w,
      vec3_cross_product(hittable->as.quad.u, planar_hitpt_vector)
  );

  if (!interval_contains(UNIT_INTERVAL, alpha) ||
      !interval_contains(UNIT_INTERVAL, beta)) {
    return false;
  }

  record->u = alpha;
  record->v = beta;

  record->t = t;
  record->p = intersection;
  record->material = hittable->material;
  hit_record_set_face_normal(record, ray, hittable->as.quad.normal);

  return true;
}

// triangle

Hittable hittable_triangle_new(
    Point3 a, Point3 b, Point3 c, struct _Material *material
) {
  Vec3 u, v, n;
  u = vec3_from_points(a, b);
  v = vec3_from_points(a, c);
  n = vec3_cross_product(u, v);

  Vec3 normal = vec3_unit_vector(n);
  Vec3 w = vec3_scalar_devide(n, vec3_dot_product(n, n));

  Aabb bounding_box = aabb_from_triangle(a, b, c);

  return (Hittable){
      .type = RAYTRACER_HITTABLE_TRIANGLE,
      .as.triangle =
          (Triangle){
              .a = a,
              .b = b,
              .c = c,
              .u = u,
              .v = v,
              .w = w,
              .normal = normal,
              .D = vec3_dot_product(normal, a),
          },
      .material = material,
      .bounding_box = bounding_box
  };
}

bool hittable_hit_triangle(
    const Hittable *hittable, Ray ray, Interval ray_t, HitRecord *record
) {
  double denom = vec3_dot_product(hittable->as.triangle.normal, ray.direction);

  if (fabs(denom) < 1e-8) {
    return false;
  }

  double t = (hittable->as.triangle.D -
              vec3_dot_product(hittable->as.triangle.normal, ray.origin)) /
             denom;

  if (!interval_contains(ray_t, t)) {
    return false;
  }

  Point3 intersection = ray_at(ray, t);
  Vec3 planar_hitpt_vector =
      vec3_subtract(intersection, hittable->as.triangle.a);
  double alpha = vec3_dot_product(
      hittable->as.triangle.w,
      vec3_cross_product(planar_hitpt_vector, hittable->as.triangle.v)
  );
  double beta = vec3_dot_product(
      hittable->as.triangle.w,
      vec3_cross_product(hittable->as.triangle.u, planar_hitpt_vector)
  );

  if (!interval_contains(UNIT_INTERVAL, alpha) ||
      !interval_contains(UNIT_INTERVAL, beta)) {
    return false;
  }

  // // this will constrain the allowed alpha and beta to triangle
  if (alpha + beta > 1.0) {
    return false;
  }

  record->u = alpha;
  record->v = beta;

  record->t = t;
  record->p = intersection;
  record->material = hittable->material;
  hit_record_set_face_normal(record, ray, hittable->as.quad.normal);

  return true;
}

// translate

Hittable hittable_translate_new(Hittable *object, Vec3 offset) {

  return (Hittable){
      .type = RAYTRACER_HITTABLE_TRANSLATE,
      .bounding_box = aabb_add_vec3(object->bounding_box, offset),
      .as.transormation = (Transformation){
          .object = object,
          .translate.offset = offset,
      }

  };
}

bool hittable_hit_translate(
    const Hittable *hittable, Ray ray, Interval ray_t, HitRecord *record
) {
  Ray offset_r = ray_new(
      vec3_subtract(ray.origin, hittable->as.transormation.translate.offset),
      ray.direction
  );

  if (!hittable_hit(
          hittable->as.transormation.object, offset_r, ray_t, record
      )) {
    return false;
  }

  record->p = vec3_add(record->p, hittable->as.transormation.translate.offset);

  return true;
}

// rotate

Hittable hittable_rotate_y_new(Hittable *object, double angle) {
  double radians = deg_to_rad(angle);
  double sin_theta = sin(radians);
  double cos_theta = cos(radians);
  Aabb bounding_box = object->bounding_box;

  Point3 min = point3_new(INFINITY, INFINITY, INFINITY);
  Point3 max = point3_new(-INFINITY, -INFINITY, -INFINITY);

  for (int i = 0; i < 2; i++) {
    for (int j = 0; j < 2; j++) {
      for (int k = 0; k < 2; k++) {
        double x = i * bounding_box.x.max + (1 - i) * bounding_box.x.min;
        double y = j * bounding_box.y.max + (1 - j) * bounding_box.y.min;
        double z = k * bounding_box.z.max + (1 - k) * bounding_box.z.min;

        double new_x = cos_theta * x + sin_theta * z;
        double new_z = -sin_theta * x + cos_theta * z;

        Vec3 tester = vec3_new(new_x, y, new_z);

        for (int c = 0; c < 3; c++) {
          min.e[c] = fmin(min.e[c], tester.e[c]);
          max.e[c] = fmax(max.e[c], tester.e[c]);
        }
      }
    }
  }

  return (Hittable){
      .type = RAYTRACER_HITTABLE_ROTATE_Y,
      .bounding_box = aabb_new(min, max),
      .as.transormation = (Transformation){
          .object = object,
          .rotate_y = (Rotate_Y){
              .cos_theta = cos_theta,
              .sin_theta = sin_theta,

          }
      }
  };
}
bool hittable_hit_rotate_y(
    const Hittable *hittable, Ray ray, Interval ray_t, HitRecord *record
) {
  double sin_theta = hittable->as.transormation.rotate_y.sin_theta;
  double cos_theta = hittable->as.transormation.rotate_y.cos_theta;

  Point3 origin = point3_new(
      (cos_theta * ray.origin.x) - (sin_theta * ray.origin.z),
      ray.origin.y,
      (sin_theta * ray.origin.x) + (cos_theta * ray.origin.z)
  );

  Vec3 direction = vec3_new(
      (cos_theta * ray.direction.x) - (sin_theta * ray.direction.z),
      ray.direction.y,
      (sin_theta * ray.direction.x) + (cos_theta * ray.direction.z)

  );

  Ray rotated_r = ray_new(origin, direction);

  if (!hittable_hit(
          hittable->as.transormation.object, rotated_r, ray_t, record
      )) {
    return false;
  }

  record->p = point3_new(
      (cos_theta * record->p.x) + (sin_theta * record->p.z),
      record->p.y,
      (-sin_theta * record->p.x) + (cos_theta * record->p.z)
  );

  record->normal = vec3_new(
      (cos_theta * record->normal.x) + (sin_theta * record->normal.z),
      record->normal.y,
      (-sin_theta * record->normal.x) + (cos_theta * record->normal.z)
  );

  return true;
}

// constant medium
//

struct _Texture;
struct _Color;

struct _Material *
material_isotropic_from_texture(struct _Texture *tex, Arena *arena);
struct _Material *material_isotropic_from_color(Vec3 color, Arena *arena);

Hittable hittable_constant_medium_from_texture(
    const Hittable *boundary, double density, struct _Texture *tex, Arena *arena
) {
  return (Hittable){
      .type = RAYTRACER_HITTABLE_CONSTANT_MEDIUM,
      .as.constant_medium =
          (ConstantMedium){
              .boundary = boundary,
              .neg_inv_density = -1.0 / density,
              .phase_function = material_isotropic_from_texture(tex, arena),
          },
      .bounding_box = boundary->bounding_box,
  };
}

Hittable hittable_constant_medium_from_color(
    const Hittable *boundary, double density, Vec3 color, Arena *arena
) {
  return (Hittable){
      .type = RAYTRACER_HITTABLE_CONSTANT_MEDIUM,
      .as.constant_medium =
          (ConstantMedium){
              .boundary = boundary,
              .neg_inv_density = -1.0 / density,
              .phase_function = material_isotropic_from_color(color, arena),
          },
      .bounding_box = boundary->bounding_box,
  };
}

bool hittable_hit_constant_medium(
    const Hittable *hittable, Ray ray, Interval ray_t, HitRecord *record
) {
  HitRecord rec1, rec2;

  if (!hittable_hit(
          hittable->as.constant_medium.boundary, ray, INTERVAL_UNIVERSE, &rec1
      )) {
    return false;
  }

  if (!hittable_hit(
          hittable->as.constant_medium.boundary,
          ray,
          interval_new(rec1.t + 0.0001, INFINITY),
          &rec2
      )) {
    return false;
  }

  if (rec1.t < ray_t.min) {
    rec1.t = ray_t.min;
  }
  if (rec2.t > ray_t.max) {
    rec2.t = ray_t.max;
  }
  if (rec1.t >= rec2.t) {
    return false;
  }
  if (rec1.t < 0) {
    rec1.t = 0;
  }

  double ray_length = vec3_length(ray.direction);
  double distance_inside_boundary = (rec2.t - rec1.t) * ray_length;
  double hit_distance =
      hittable->as.constant_medium.neg_inv_density * log(random_double());

  if (hit_distance > distance_inside_boundary) {
    return false;
  }

  record->t = rec1.t + hit_distance / ray_length;
  record->p = ray_at(ray, record->t);

  record->normal = vec3_new(1, 0, 0);
  record->front_face = true;
  record->material = hittable->as.constant_medium.phase_function;

  return true;
}

bool hittable_hit(
    const Hittable *hittable, Ray ray, Interval ray_t, HitRecord *record
) {
  switch (hittable->type) {
  case RAYTRACER_HITTABLE_SPHERE:
    return hittable_hit_sphere(hittable, ray, ray_t, record);
  case RAYTRACER_HITTABLE_QUAD:
    return hittable_hit_quad(hittable, ray, ray_t, record);
  case RAYTRACER_HITTABLE_TRIANGLE:
    return hittable_hit_triangle(hittable, ray, ray_t, record);
  case RAYTRACER_HITTABLE_CONSTANT_MEDIUM:
    return hittable_hit_constant_medium(hittable, ray, ray_t, record);
  case RAYTRACER_HITTABLE_COLLECTION:
    return hittable_hit_list(hittable, ray, ray_t, record);
  case RAYTRACER_HITTABLE_BHV_NODE:
    return hittable_hit_bhv_node(hittable, ray, ray_t, record);
  case RAYTRACER_HITTABLE_TRANSLATE:
    return hittable_hit_translate(hittable, ray, ray_t, record);
  case RAYTRACER_HITTABLE_ROTATE_Y:
    return hittable_hit_rotate_y(hittable, ray, ray_t, record);
  }
}

#endif

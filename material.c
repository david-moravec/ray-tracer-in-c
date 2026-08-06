#ifndef RAYTRACER_MATERIAL_C
#define RAYTRACER_MATERIAL_C

#include "stdbool.h"

#include "color.c"
#include "hittable.c"
#include "ray.c"
#include "vec3.c"

typedef enum {
  RAYTRACER_MATERIAL_LAMBERTIAN,
  RAYTRACER_MATERIAL_METAL
} MaterialType;

typedef struct _Material {
  MaterialType type;
  Color albedo;
  double fuzz;
} Material;

Material material_new(MaterialType type, Color color, double fuzz) {
  return (Material){.type = type, .albedo = color, .fuzz = fuzz};
}

bool material_scatter_lambertian(Material *material, Ray ray_in,
                                 HitRecord record, Color *attenuation,
                                 Ray *scattered) {
  Vec3 scatter_direction = vec3_add(record.normal, random_unit_vector());

  if (vec3_near_zero(scatter_direction)) {
    scatter_direction = record.normal;
  }

  *scattered = ray_new(record.p, scatter_direction);
  *attenuation = material->albedo;

  return true;
}

bool material_scatter_metal(Material *material, Ray ray_in, HitRecord record,
                            Color *attenuation, Ray *scattered) {
  Vec3 reflected = vec3_reflect(ray_in.direction, record.normal);
  reflected =
      vec3_add(vec3_unit_vector(reflected),
               vec3_scalar_multiply(random_unit_vector(), material->fuzz));
  *scattered = ray_new(record.p, reflected);
  *attenuation = material->albedo;

  return true;
}

bool material_scatter(Material *material, Ray ray_in, HitRecord record,
                      Color *attenuation, Ray *scattered) {
  switch (material->type) {
  case RAYTRACER_MATERIAL_LAMBERTIAN: {
    return material_scatter_lambertian(material, ray_in, record, attenuation,
                                       scattered);
  }
  case RAYTRACER_MATERIAL_METAL: {
    return material_scatter_metal(material, ray_in, record, attenuation,
                                  scattered);
  }
  }
};

#endif

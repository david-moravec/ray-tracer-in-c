#ifndef RAYTRACER_MATERIAL_C
#define RAYTRACER_MATERIAL_C

#include "base/include/arena.h"
#include "rtcommon.h"
#include "stdbool.h"

#include "color.c"
#include "hittable.c"
#include "ray.c"
#include "texture.c"
#include "vec3.c"

typedef enum {
  RAYTRACER_MATERIAL_LAMBERTIAN,
  RAYTRACER_MATERIAL_METAL,
  RAYTRACER_MATERIAL_DIELECTRICS,
  RAYTRACER_MATERIAL_DIFFUSE_LIGHT,
} MaterialType;

typedef struct _Material {
  MaterialType type;
  union {
    Color albedo;
    Texture *texture;
  };
  double fuzz;
  double refraction_index;
} Material;

Material material_lambertian_from_color(Color color, Arena *arena) {

  Texture texture = texture_solid_color_from_color(color);
  return (Material){.type = RAYTRACER_MATERIAL_LAMBERTIAN,
                    .texture = ARENA_PUSH_COPY(arena, Texture, &texture)};
}

Material material_lambertian_from_texture(Texture *texture) {

  return (Material){.type = RAYTRACER_MATERIAL_LAMBERTIAN, .texture = texture};
}

Material material_metal_new(Color color, double fuzz) {
  return (Material){
      .type = RAYTRACER_MATERIAL_METAL, .albedo = color, .fuzz = fuzz};
}

Material material_dielectrics_new(double refraction_index) {
  return (Material){.type = RAYTRACER_MATERIAL_DIELECTRICS,
                    .refraction_index = refraction_index};
}

bool material_scatter_lambertian(Material *material, Ray ray_in,
                                 HitRecord record, Color *attenuation,
                                 Ray *scattered) {
  Vec3 scatter_direction = vec3_add(record.normal, random_unit_vector());

  if (vec3_near_zero(scatter_direction)) {
    scatter_direction = record.normal;
  }

  *scattered = ray_new(record.p, scatter_direction);
  *attenuation = texture_value(material->texture, record.u, record.v, record.p);

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

static inline double reflectance(double cosine, double refraction_index) {
  double r0 = (1 - refraction_index) / (1 + refraction_index);
  r0 = r0 * r0;
  return r0 + (1 - r0) * pow(1 - cosine, 5);
}

bool material_scatter_dielectrics(Material *material, Ray ray_in,
                                  HitRecord record, Color *attenuation,
                                  Ray *scattered) {
  *attenuation = color_new(1.0, 1.0, 1.0);
  double ri = record.front_face ? (1.0 / material->refraction_index)
                                : material->refraction_index;

  Vec3 unit_direction = vec3_unit_vector(ray_in.direction);
  double cos_theta =
      fmin(vec3_dot_product(vec3_negative(unit_direction), record.normal), 1.0);
  double sin_theta = sqrt(1.0 - cos_theta * cos_theta);

  bool cannot_refract = ri * sin_theta > 1.0;
  Vec3 direction = {0};

  if (cannot_refract || reflectance(cos_theta, ri) > random_double()) {
    direction = vec3_reflect(unit_direction, record.normal);
  } else {
    direction = vec3_refract(unit_direction, record.normal, ri);
  }

  *scattered = ray_new(record.p, direction);

  return true;
}

// diffuse light
//

Material material_diffuse_light_new(Color emit, Arena *arena) {
  Texture text = texture_solid_color_from_color(emit);

  return (Material){.type = RAYTRACER_MATERIAL_DIFFUSE_LIGHT,
                    .texture = ARENA_PUSH_COPY(arena, Texture, &text)};
}

static Color diffuse_light_emitted(Material *material, double u, double v,
                                   Point3 p) {
  return texture_value(material->texture, u, v, p);
}

bool material_scatter(Material *material, Ray ray_in, HitRecord record,
                      Color *attenuation, Ray *scattered) {
  switch (material->type) {
  case RAYTRACER_MATERIAL_LAMBERTIAN:
    return material_scatter_lambertian(material, ray_in, record, attenuation,
                                       scattered);
  case RAYTRACER_MATERIAL_METAL:
    return material_scatter_metal(material, ray_in, record, attenuation,
                                  scattered);
  case RAYTRACER_MATERIAL_DIELECTRICS:
    return material_scatter_dielectrics(material, ray_in, record, attenuation,
                                        scattered);
  case RAYTRACER_MATERIAL_DIFFUSE_LIGHT:
    return false;
  }
};

Color material_emitted(Material *material, double u, double v, Point3 p) {
  switch (material->type) {
  case RAYTRACER_MATERIAL_LAMBERTIAN:
  case RAYTRACER_MATERIAL_METAL:
  case RAYTRACER_MATERIAL_DIELECTRICS:
    return color_new(0, 0, 0);
  case RAYTRACER_MATERIAL_DIFFUSE_LIGHT:
    return diffuse_light_emitted(material, u, v, p);
  }
}

#endif

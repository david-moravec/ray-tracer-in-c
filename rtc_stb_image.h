#ifndef RAYTRACER_RTC_STB_IMAGE_H
#define RAYTRACER_RTC_STB_IMAGE_H

#define STB_IMAGE_IMPLEMENTATION
#define STBI_FAILURE_USERMSG
#include "external/stb_image.h"

#include "base/include/arena.h"
#include "stdio.h"
#include "stdlib.h"

const unsigned char MAGENTA[] = {255, 0, 255};

typedef struct {
  int bytes_per_pixel;
  float *fdata;
  unsigned char *bdata;
  int width;
  int height;
  int bytes_per_scanline;
} RTCImage;

static unsigned char float_to_byte(float value) {
  if (value <= 0.0) {
    return 0;
  }
  if (1.0 <= value) {
    return 255;
  }
  return (unsigned char)(256.0 * value);
}

static void convert_to_bytes(RTCImage *image, Arena *arena) {
  int total_bytes = image->width * image->height * image->bytes_per_pixel;
  image->bdata = (unsigned char *)arena_push(arena, sizeof(char) * total_bytes);

  unsigned char *bptr = image->bdata;
  float *fptr = image->fdata;

  for (int i = 0; i < total_bytes; i++, fptr++, bptr++) {
    *bptr = float_to_byte(*fptr);
  }
}

static int clamp(int i, int low, int high) {
  if (i < low) {
    return low;
  }
  if (i < high) {
    return i;
  }

  return high - 1;
}

static const unsigned char *rtc_image_pixel_data(RTCImage *image, int x,
                                                 int y) {
  if (image->bdata == NULL) {
    return MAGENTA;
  }

  x = clamp(x, 0, image->width);
  y = clamp(y, 0, image->height);

  return image->bdata + y * image->bytes_per_scanline +
         x * image->bytes_per_pixel;
}

static bool rtc_image_load(RTCImage *image, const char *image_path,
                           Arena *arena) {
  int n = image->bytes_per_pixel;
  image->fdata = stbi_loadf(image_path, &image->width, &image->height, &n,
                            image->bytes_per_pixel);
  if (image->fdata == NULL) {
    return false;
  }

  image->bytes_per_scanline = image->width * image->bytes_per_pixel;
  convert_to_bytes(image, arena);

  return true;
}

static RTCImage rtc_image_new(const char *path, Arena *arena) {
  RTCImage result = {0};
  result.bytes_per_pixel = 3;
  rtc_image_load(&result, path, arena);
  return result;
}

#endif

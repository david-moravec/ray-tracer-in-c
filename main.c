#include "stdint.h"
#include "stdio.h"
#include <stdint.h>

#include "color.h"

typedef float f32;
typedef double f64;

int main() {
  char buff[BUFSIZ];
  setvbuf(stderr, buff, _IOFBF, BUFSIZ);

  uint16_t image_width = 256;
  uint16_t image_height = 256;

  printf("P3\n%u %u\n255\n", image_width, image_height);

  for (int j = 0; j < image_height; j++) {
    fprintf(stderr, "\rScanlines reamining: %-6u", (image_height - j));
    fflush(stderr);
    for (int i = 0; i < image_width; i++) {
      Color pixel_color = {.x = (f64)i / (f64)(image_width - 1),
                           .y = (f64)j / (f64)(image_height - 1),
                           .z = 0.0};

      color_fprint(stdout, &pixel_color);
    }
  }
  fprintf(stderr, "\r%-40s\n", "Done");
  fflush(stderr);
}

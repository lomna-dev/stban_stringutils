#define STBAN_BMP_IMPLEMENTATION
#include "../../stban_bmp.h"

#include <stdint.h>
#include <stdlib.h>
#include <math.h>

int main()
{
  size_t w, h;
  stban_bmp_color *map = stban_bmp_read("input/input.bmp", &w, &h);
  stban_bmp_write(map, w, h, "output.bmp");
  return 0;
}

#define STBAN_BMP_IMPLEMENTATION
#include "../../stban_bmp.h"

#include <stdint.h>
#include <stdlib.h>
#include <math.h>

void grayscale(stban_bmp_color **map, size_t no_pixels)
{
  for(size_t i = 0; i < no_pixels; i++)
    {
      uint8_t avg = (map[0][i].R + map[0][i].G + map[0][i].B) / 3;
      map[0][i].R = avg;
      map[0][i].G = avg;
      map[0][i].B = avg;
    }
}

void color_filter(stban_bmp_color **map, size_t no_pixels, stban_bmp_color color, size_t intensity)
{
  for(size_t j = 0; j < intensity; j++)
    {
      for(size_t i = 0; i < no_pixels; i++)
	{
	  map[0][i].R = (map[0][i].R + color.R) / 2;
	  map[0][i].G = (map[0][i].G + color.G) / 2;
	  map[0][i].B = (map[0][i].B + color.B) / 2;
	}
    }
}

int main()
{
  size_t w, h;
  stban_bmp_color *map = stban_bmp_read("input/input.bmp", &w, &h);
  /* grayscale(&map, w * h); */
  color_filter(&map, w * h, stban_bmp_ARGB(255, 100, 150, 200), 2);
  stban_bmp_write(map, w, h, "output.bmp");
  return 0;
}

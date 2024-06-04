#define STBAN_BMP_IMPLEMENTATION
#include "../../stban_bmp.h"

#include <stdint.h>
#include <stdlib.h>
#include <math.h>

#define WIDTH 7680
#define HEIGHT 4320

// return between 1 and 255*iteration_factor for elements definitely not in set
// 1-(255*i) means number of iterations to escape
// return -1 for numbers probably in set
int mandelbrot(double x0, double y0){
  const int iteration_factor = 5;
  const int max_iterations = 255 * iteration_factor;

  double x = 0;
  double y = 0;

  int iteration = 0;
  while((x*x + y*y) <= 4.0 && iteration <= max_iterations){
    double xtemp = x*x - y*y + x0;
    y = 2*x*y + y0;
    x = xtemp;
    iteration++;
  }

  if(iteration > max_iterations)
    return -1;
  
  return iteration;
}

stban_bmp_color get_color(size_t x, size_t y) {
  int i = mandelbrot(-2.3 + 3.0*(x*1.0/WIDTH), -1.0 + 2.0*(y*1.0/HEIGHT));

  if(i < 0)
    return stban_bmp_ARGB(255,0,0,0);

  // adds glow around edges
  // useful since we are using black to identify elements of set
  // so increasing glow helps us know elements actually in set
  i = i * log(i);
  if(i > 255)
    i = 255;

#define GRAYSCALE
#ifdef GRAYSCALE
  return stban_bmp_ARGB(255, (uint8_t) 200 - (i + 25),
			(uint8_t) 200 - (i + 25), (uint8_t) 200 - (i + 25));
#else
  return (stban_bmp_color) {
    .R = (uint8_t) 255 - (i),
    .G = (uint8_t) 200 - (i + 55),
    .B = (uint8_t) 200 - (i + 55),
    .A = 255
  };
#endif
}

int main(){
  stban_bmp_color *map = (stban_bmp_color *) malloc(sizeof(stban_bmp_color) * (HEIGHT*WIDTH));

  for(int i = 0; i < HEIGHT; i++){
    for(int j = 0; j < WIDTH; j++){
      map[j + i*WIDTH] = get_color(j,i);
    }
  }
  
  stban_bmp_write(map, WIDTH, HEIGHT, "mandelbrot.bmp");
  free(map);
  return 0;
}

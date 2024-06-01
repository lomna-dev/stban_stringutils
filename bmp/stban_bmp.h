#ifndef STBAN_BMP_INCLUDED_H
#define STBAN_BMP_INCLUDED_H

#include <stdio.h>
#include <stdint.h>

typedef struct stban_bmp_color stban_bmp_color;
struct stban_bmp_color {
  uint8_t B;
  uint8_t G;
  uint8_t R;
  uint8_t A;
};

typedef struct stban_bmp_file stban_bmp_file;
struct stban_bmp_file {
  char *data;
  size_t len;
  size_t capacity;
};

#define stban_bmp_ARGB(a,r,g,b) ((stban_bmp_color){.B=(b),.G=(g),.R=(r),.A=(a)})

void stban_bmp_write(stban_bmp_color *map, size_t w, size_t h, char *output);

#endif // STBAN_BMP_INCLUDED_H

#ifdef STBAN_BMP_IMPLEMENTATION

stban_bmp_file stban_bmp_file_read_file(const char *filename)
{
  FILE *f = fopen(filename, "rb");
  if(f == NULL)
    {
      fprintf(stderr, "Failed to open file : %s\n", filename);
      perror("Error");
      exit(EXIT_FAILURE);
    }
  
  if (fseek(f, 0, SEEK_END) < 0)
    {
      fprintf(stderr, "Failed to read from file : %s\n", filename);
      perror("Error");
      exit(EXIT_FAILURE);
    }
  
  long fsize = ftell(f);
  if (fsize < 0)
    {
      fprintf(stderr, "Failed to read from file : %s\n", filename);
      perror("Error");
      exit(EXIT_FAILURE);
    }
  
  if (fseek(f, 0, SEEK_SET) < 0)
    {
      fprintf(stderr, "Failed to read from file : %s\n", filename);
      perror("Error");
      exit(EXIT_FAILURE);
    }

  char *buffer = (char *) malloc(fsize * sizeof(char));
  STBAN_STRING_ASSERT((buffer != NULL) && "malloc failed, buy more ram");
  
  fread(buffer, fsize, 1, f);
  if (ferror(f))
    {
      fprintf(stderr, "Failed to read from file : %s\n", filename);
      perror("Error");
      exit(EXIT_FAILURE);
    }
  
  stban_bmp_file sb = (stban_bmp_file) {
    .data = buffer,
    .len = fsize,
    .capacity = fsize
  };
  
  fclose(f);

  return sb;
}

stban_bmp_color *stban_bmp_read()
{

}

// works on little-endian machines (not tested on big-endian machines)
void stban_bmp_write(stban_bmp_color *map, size_t w, size_t h, char *output) {
  char file_size[4] = {
    (122 + (w*h)) & 0xFF,
    ((122 + (w*h)) >> 8) & 0xFF,
    ((122 + (w*h)) >> 16) & 0xFF,
    ((122 + (w*h)) >> 32) & 0xFF
  };
  
  char map_size[4] = {
    (w*h) & 0xFF,
    ((w*h) >> 8) & 0xFF,
    ((w*h) >> 16) & 0xFF,
    ((w*h) >> 32) & 0xFF
  };
  
  char map_width[4] = {
    w & 0xFF,
    (w >> 8) & 0xFF,
    (w >> 16) & 0xFF,
    (w >> 32) & 0xFF
  };
  
  char map_height[4] = {
    h & 0xFF,
    (h >> 8) & 0xFF,
    (h >> 16) & 0xFF,
    (h >> 32) & 0xFF
  };

  char BMP_header[14] = {'B', 'M',
			 file_size[0], file_size[1], file_size[2], file_size[3],
			 0x00, 0x00,
			 0x00, 0x00,
			 0x7A, 0x00, 0x00, 0x00
  };

  char DIB_header[60] = {
    0x6C, 0x00, 0x00, 0x00,
    map_width[0], map_width[1], map_width[2], map_width[3],
    map_height[0], map_height[1], map_height[2], map_height[3],
    0x01, 0x00,
    0x20, 0x00,
    0x03, 0x00, 0x00, 0x00,
    map_size[0], map_size[1], map_size[2], map_size[3],
    0x13, 0x0B, 0x00, 0x00,
    0x13, 0x0B, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0xFF, 0x00,
    0x00, 0xFF, 0x00, 0x00,
    0xFF, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0xFF,
    0x20, 0x6E, 0x69, 0x57
  };

  char DIB_header_padding[48] = {0};
  
  FILE *fptr = fopen(output, "wb");
  
  fwrite(BMP_header, sizeof(char), 14, fptr);
  fwrite(DIB_header, sizeof(char), 60, fptr);
  fwrite(DIB_header_padding, sizeof(char), 48, fptr);

  for(int i = 0; i < (w*h); i++){
    fwrite(&map[i], sizeof(char), 4, fptr);
  }
  
  fclose(fptr);
}

#endif // STBAN_BMP_IMPLEMENTATION

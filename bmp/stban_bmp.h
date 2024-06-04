#ifndef STBAN_BMP_INCLUDED_H
#define STBAN_BMP_INCLUDED_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define STBAN_BMP_ASSERT(X) do{if(!(X)) ((char*)NULL)[0] = 0; }while(0)

typedef struct stban_bmp_color stban_bmp_color;
struct stban_bmp_color {
  uint8_t B;
  uint8_t G;
  uint8_t R;
  uint8_t A;
};

#define stban_bmp_ARGB(a,r,g,b) ((stban_bmp_color){.B=(b),.G=(g),.R=(r),.A=(a)})

void stban_bmp_write(stban_bmp_color *map, size_t w, size_t h, char *output);
stban_bmp_color *stban_bmp_read(const char *filename, size_t *width, size_t *height);

#endif // STBAN_BMP_INCLUDED_H

#ifdef STBAN_BMP_IMPLEMENTATION

typedef struct stban_bmp_file stban_bmp_file;
struct stban_bmp_file {
  uint8_t *data;
  size_t len;
  uint32_t width;
  uint32_t height;
  uint32_t compression;
  uint16_t bits_per_pixel;
  uint32_t bitmap_size;
  uint32_t offset;
};

stban_bmp_file stban_bmp_file_read_file(const char *filename)
{
  FILE *f = fopen(filename, "rb");
  if(f == NULL)
    {
      fprintf(stderr, "Failed to open file : %s\n", filename);
      perror("Error");
      /* exit(EXIT_FAILURE); */
    }
  
  if (fseek(f, 0, SEEK_END) < 0)
    {
      fprintf(stderr, "Failed to read from file : %s\n", filename);
      perror("Error");
      /* exit(EXIT_FAILURE); */
    }
  
  long fsize = ftell(f);
  if (fsize < 0)
    {
      fprintf(stderr, "Failed to read from file : %s\n", filename);
      perror("Error");
      /* exit(EXIT_FAILURE); */
    }
  
  if (fseek(f, 0, SEEK_SET) < 0)
    {
      fprintf(stderr, "Failed to read from file : %s\n", filename);
      perror("Error");
      /* exit(EXIT_FAILURE); */
    }

  uint8_t *buffer = (uint8_t *) malloc(fsize * sizeof(uint8_t));
  /* STBAN_BMP_ASSERT((buffer[0] == 0x42) && (buffer[1] == 0x4d) && "Not a bmp file"); */
  
  fread(buffer, fsize, 1, f);
  if (ferror(f))
    {
      fprintf(stderr, "Failed to read from file : %s\n", filename);
      perror("Error");
      /* exit(EXIT_FAILURE); */
    }

  size_t width  = ((size_t)buffer[18])       | ((size_t)buffer[19] << 8) |
                  ((size_t)buffer[20] << 16) | ((size_t)buffer[21] << 32);
  size_t height = ((size_t)buffer[22])       | ((size_t)buffer[23] << 8) |
                  ((size_t)buffer[24] << 16) | ((size_t)buffer[25] << 32);

  size_t bits_per_pixel = ((size_t)buffer[28]) | ((size_t)buffer[29] << 8);
  size_t compression = ((size_t)buffer[30])       | ((size_t)buffer[31] << 8) |
                       ((size_t)buffer[32] << 16) | ((size_t)buffer[33] << 32);

  size_t offset = ((size_t)buffer[10])       | ((size_t)buffer[11] << 8) |
                  ((size_t)buffer[12] << 16) | ((size_t)buffer[13] << 32);
  
  size_t bitmap_size = ((size_t)buffer[34])       | ((size_t)buffer[35] << 8) |
                       ((size_t)buffer[36] << 16) | ((size_t)buffer[37] << 32);
    
  stban_bmp_file sb = (stban_bmp_file) {
    .data	    = buffer,
    .len	    = fsize,
    .width	    = (uint32_t) width,
    .height	    = (uint32_t) height,
    .bits_per_pixel = (uint16_t) bits_per_pixel,
    .compression    = (uint32_t) compression,
    .offset	    = (uint32_t) offset,
    .bitmap_size    = (uint32_t) bitmap_size
  };
  
  fclose(f);

  return sb;
}

void stban_bmp_rgb_map_from_file(stban_bmp_file file, stban_bmp_color **map)
{
  uint16_t bytes_per_pixel = file.bits_per_pixel / 8;
  if(file.bitmap_size == 0)
    {
      size_t file_size = ((size_t)file.data[2])       | ((size_t)file.data[3] << 8) |
	                 ((size_t)file.data[4] << 16) | ((size_t)file.data[5] << 32);
      file.bitmap_size = file_size - file.offset;
    }
  
  uint32_t padding_size = file.bitmap_size - (file.height * file.width * bytes_per_pixel);
  uint32_t padding_bytes = padding_size / file.height;
  
  size_t current = file.offset;
  for(uint32_t y = 0; y < file.height; y++)
    {
      for(uint32_t x = 0; x < file.width; x++)
	{
	  map[0][x + file.width * y] = stban_bmp_ARGB(255, file.data[current + 2],
						      file.data[current + 1], file.data[current]);
	  current += bytes_per_pixel;
	}
      current += padding_bytes;
    }
}

void stban_bmp_argb_map_from_file(stban_bmp_file file, stban_bmp_color **map)
{
  uint16_t bytes_per_pixel = file.bits_per_pixel / 8;
  size_t current = file.offset;

  // find masks and then extract
  STBAN_BMP_ASSERT(0 && "TODO : NOT IMPLEMENTED stban_bmp_argb_map_from_file");
  /* for(uint32_t y = 0; y < file.height; y++) */
  /*   { */
  /*     for(uint32_t x = 0; x < file.width; x++) */
  /* 	{ */
  /* 	  map[0][x + file.width * y] = stban_bmp_ARGB(255, file.data[current + 2], */
  /* 						      file.data[current + 1], file.data[current]); */
  /* 	  current += bytes_per_pixel; */
  /* 	} */
  /*   } */
}

stban_bmp_color *stban_bmp_read(const char *filename, size_t *width, size_t *height)
{
  stban_bmp_file file = stban_bmp_file_read_file(filename);
  stban_bmp_color *map = malloc(file.width * file.height * sizeof(stban_bmp_color));
  STBAN_BMP_ASSERT((map != NULL) && "malloc failed buy more ram lol");

  width[0] = file.width;
  height[0] = file.height;

  if(file.compression == 0x0000)
    stban_bmp_rgb_map_from_file(file, &map);
  else if(file.compression == 0x0003)
    stban_bmp_argb_map_from_file(file, &map);
  else
    printf("[ERROR] stban_bmp_read : unknown compression\n");
  
  return map;
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

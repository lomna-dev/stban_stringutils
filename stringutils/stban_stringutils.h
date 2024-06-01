#ifndef STBAN_STRINGUTILS_INCLUDED_H
#define STBAN_STRINGUTILS_INCLUDED_H

/* define to use stban_sb_read_file  */
/* adds stdio.h */
/* #define STBAN_STRING_IO */

/* #include <assert.h> */

/* This assert causes segfault to read stack trace
 * Don't use this on systems with no segfaults 
 * or in systems where NULL can be written to
 */
#define STBAN_STRING_ASSERT(X) do{if(!(X)) ((char*)NULL)[0] = 0; }while(0)

#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

//* string views *//
typedef struct stban_sv stban_sv;
struct stban_sv {
  char *data;
  size_t len;
};

//* creating stringview *//
stban_sv stban_sv_from_cstr(char *data);
stban_sv stban_sv_from_substr(char *data, size_t from, size_t to);

//* trim whitespaces *//
// returns the number of characters that were trimmed
size_t stban_sv_trim_left(stban_sv *sv);
size_t stban_sv_trim_right(stban_sv *sv);
size_t stban_sv_trim(stban_sv *sv);

//* comprisions stringviews *//
int stban_sv_cmp(stban_sv a, stban_sv b);
#define stban_sv_eq(a,b) !stban_sv_cmp(a,b)
bool stban_sv_starts_with(stban_sv sv, stban_sv prefix);
bool stban_sv_ends_with(stban_sv sv, stban_sv suffix);

//* chopping stringviews *//
stban_sv stban_sv_chop(stban_sv *sv, size_t n);
stban_sv stban_sv_chop_by_delim(stban_sv *sv, char delim);
stban_sv stban_sv_chop_by_sv(stban_sv *sv, stban_sv delim);

//* string builder *//
typedef struct stban_sb stban_sb;
struct stban_sb {
  char *data;
  size_t len;
  size_t capacity;
};

//* creating string builders *//
stban_sb stban_sb_create();
stban_sb stban_sb_from_cstr(char *data);
stban_sb stban_sb_from_substr(char *data, size_t from, size_t to);
void stban_sb_delete(stban_sb sb);

//* appending to string builder *//
void stban_sb_append_substr(stban_sb *sb, char *str, size_t from, size_t to);
void stban_sb_append_cstr(stban_sb *sb, char *cstr);

#define stban_sb_clear(sb) sb.len = 0

//* conversion to cstring *//
// does not increase len of the string builder buffer
// will increase capacity if not enough space
void stban_sb_append_null(stban_sb *sb);

//* chopping string builders *//
// the right string buffer is chopped and the resulting part is appended to left

// before chop operation
// | left    | right   |
// |---------+---------|
// | "Hello" | "World" |

// after chop by delim 'r'
// | left     | right |
// |----------+-------|
// | "HelloWo"| "ld"  |
void stban_sb_chop(stban_sb *left, stban_sb *right, size_t n);
void stban_sb_chop_by_delim(stban_sb *left, stban_sb *right, char delim);
void stban_sb_chop_by_substr(stban_sb *left, stban_sb *right,
			     char *str, size_t from, size_t to);

//* macros that depend on both string view and string builder *//
#define stban_sv_from_sb(sb) ((stban_sv) {.data = sb.data, .len =  sb.len} )
#define stban_sb_from_sv(sv) stban_sb_from_substr(sv.data, 0, sv.len)
#define stban_sb_append_sv(pointer_to_sb, sv) stban_sb_append_substr(pointer_to_sb, sv.data, 0, sv.len)

#define STB_STR_FMT "%.*s"
#define STB_STR_ARG(svb) (int) (svb).len, (svb).data
// USAGE:
//   stban_sv name = ...; OR
//   stban_sb name = ...;
//   printf("Name: " STB_STR_FMT "\n", STB_STR_ARG(name));


#ifdef STBAN_STRING_IO
#include <stdio.h>
stban_sb stban_sb_read_file(const char *filename);
#endif

#endif // STBAN_STRINGUTILS_INCLUDED_H

#ifdef STBAN_STRINGUTILS_IMPLEMENTATION

stban_sv stban_sv_from_substr(char *data, size_t from, size_t to) {
  stban_sv sv;
  sv.data = (data + from);
  STBAN_STRING_ASSERT((to >= from) && "from needs to be greater than too :: stban_sv_from_substr");
  sv.len = to - from;
  return sv;
}

inline
stban_sv stban_sv_from_cstr(char *data){
  return stban_sv_from_substr(data, 0, strlen(data));
}

int stban_sv_cmp(stban_sv a, stban_sv b) {
  if(a.len > b.len)
    return (a.data[b.len] - 0);
  else if(a.len < b.len)
    return (0 - b.data[a.len]);
  
  return strncmp(a.data, b.data, a.len);
}

size_t stban_sv_trim_left(stban_sv *sv) {
  size_t i = 0;
  while(i < sv[0].len && isspace(sv[0].data[i]))
    i++;

  sv[0].data = (sv[0].data + i);
  sv[0].len = sv[0].len - i;
  
  return i;
}

size_t stban_sv_trim_right(stban_sv *sv) {
  size_t i = 0;
  while(i < sv[0].len && isspace(sv[0].data[sv[0].len - 1 - i]))
    i++;

  sv[0].len = sv[0].len - i;

  return i;
}

size_t stban_sv_trim(stban_sv *sv) {
  return (stban_sv_trim_left(sv) + stban_sv_trim_right(sv));
}

stban_sv stban_sv_chop(stban_sv *sv, size_t n) {
  STBAN_STRING_ASSERT((n < sv[0].len && n >= 0) && "Index out of range :: stban_sv_chop");
  stban_sv result = stban_sv_from_substr(sv[0].data, 0, n);
  sv[0] = stban_sv_from_substr(sv[0].data, n + 1, sv[0].len);
  return result;
}

stban_sv stban_sv_chop_by_delim(stban_sv *sv, char delim){
  size_t i = 0;
  stban_sv result = {
    .data = NULL,
    .len = 0
  };
  
  for(i = 0; i < sv[0].len; i++){
    if(sv[0].data[i] == delim){
      result = stban_sv_chop(sv, i);
      break;
    }
  }

  return result;
}

stban_sv stban_sv_chop_by_sv(stban_sv *sv, stban_sv delim){
  size_t i = 0;
  stban_sv result = {
    .data = NULL,
    .len = 0
  };

  if(sv[0].len < delim.len)
    return result;
    
  stban_sv window = stban_sv_from_substr(sv[0].data, 0, delim.len);
  for(i = 0; i < (sv[0].len - delim.len + 1); i++){
    if(stban_sv_eq(window, delim)){
      result = stban_sv_chop(sv, i);
      sv[0] = stban_sv_from_substr(sv[0].data, window.len - 1, sv[0].len);
      break;
    }
    window.data += 1;
  }
  
  return result;
}

inline
bool stban_sv_starts_with(stban_sv sv, stban_sv prefix) {
  return ((sv.len > prefix.len) &&
	  (stban_sv_eq(prefix, stban_sv_from_substr(sv.data, 0, prefix.len))));
}

inline
bool stban_sv_ends_with(stban_sv sv, stban_sv suffix) {
  return ((sv.len > suffix.len) &&
	  (stban_sv_eq(suffix, stban_sv_from_substr(sv.data, sv.len - suffix.len, sv.len))));
}


stban_sb stban_sb_create(){
  stban_sb sb;
  sb.capacity = 16;
  sb.data = (char *) malloc(sb.capacity * sizeof(char));
  STBAN_STRING_ASSERT((sb.data != NULL) && "malloc failed for string builder :: stban_sb_create");
  sb.len = 0;
  return sb;  
}

stban_sb stban_sb_from_substr(char *str, size_t from, size_t to){
  STBAN_STRING_ASSERT((to >= from) && "range invalid, to must be greater than or equal to from :: stban_sb_from_substr");

  stban_sb sb;
  
  size_t count = to - from;
  size_t capacity = 16;
  while(capacity <= count)
    capacity = capacity << 1;

  sb.capacity = capacity;
  sb.len = to - from;
  sb.data = (char *) malloc(sb.capacity * sizeof(char));
  STBAN_STRING_ASSERT((sb.data != NULL) && "malloc failed for string builder :: stban_sb_from_substr");

  sb.data = (char *) memcpy(sb.data, str + from, sb.len * sizeof(char));

  return sb;
}

inline
stban_sb stban_sb_from_cstr(char *data){
  return stban_sb_from_substr(data, 0, strlen(data));
}

inline
void stban_sb_delete(stban_sb sb){
  free(sb.data);
}

void stban_sb_append_substr(stban_sb *sb, char *str, size_t from, size_t to) {
  STBAN_STRING_ASSERT((to >= from) && "range invalid, to must be greater than or equal to from :: stban_sb_append_substr");
  
  size_t count = to - from;
  size_t capacity = sb[0].capacity;
  while(capacity <= (count + sb[0].len))
    capacity = capacity << 1;

  if(capacity != sb[0].capacity){
    sb[0].capacity = capacity;
    sb[0].data = (char *) realloc(sb[0].data, sb[0].capacity * sizeof(char));
    STBAN_STRING_ASSERT((sb[0].data != NULL) && "realloc failed for string builder :: stban_sb_append_substr");
  }

  memcpy(sb[0].data + sb[0].len, str + from, count * sizeof(char));
  sb[0].len = count + sb[0].len;
}

inline
void stban_sb_append_cstr(stban_sb *sb, char *cstr) {
  stban_sb_append_substr(sb, cstr, 0, strlen(cstr));
}

void stban_sb_append_null(stban_sb *sb){
  if(sb[0].len >= sb[0].capacity){
    sb[0].capacity = sb[0].capacity << 1;
    sb[0].data = (char *) realloc(sb[0].data, sb[0].capacity * sizeof(char));
    STBAN_STRING_ASSERT((sb[0].data != NULL) && "realloc failed for string builder :: stban_sb_append_null");
  }
  sb[0].data[sb[0].len] = '\0';
}

void stban_sb_chop(stban_sb *left, stban_sb *right, size_t n) {
  STBAN_STRING_ASSERT((n >= 0 && n < right[0].len) && "index out of range :: stban_sb_chop");
  stban_sb_append_substr(left, right[0].data, 0, n);
  right[0].data = (char *) memmove(right[0].data, right[0].data + n + 1, right[0].len - n - 1);
  STBAN_STRING_ASSERT((right[0].data != NULL) && "memmove failed :: stban_sb_chop");
  right[0].len = right[0].len - n - 1;
}

void stban_sb_chop_by_substr(stban_sb *left, stban_sb *right,
			     char *str, size_t from, size_t to){
  stban_sv right_sv = stban_sv_from_sb(right[0]);
  stban_sv left_sv = stban_sv_chop_by_sv(&right_sv, (stban_sv){.data = str, .len = (to - from)});

  stban_sb_append_sv(left, left_sv);
  right[0].data = (char *) memmove(right[0].data, right_sv.data, right_sv.len * sizeof(char));
  right[0].len = right_sv.len;
}

#ifdef STBAN_STRING_IO
stban_sb stban_sb_read_file(const char *filename)
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
  
  stban_sb sb = (stban_sb) {
    .data = buffer,
    .len = fsize,
    .capacity = fsize
  };
  
  fclose(f);

  return sb;
}
#endif

#endif // STBAN_STRINGUTILS_IMPLEMENTATION

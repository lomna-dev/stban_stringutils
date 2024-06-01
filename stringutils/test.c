#define STBAN_STRINGUTILS_IMPLEMENTATION
#define STBAN_STRING_IO
#include "./stban_stringutils.h"

int main() {
  stban_sb left = stban_sb_from_cstr("Hello World");
  stban_sb right = stban_sb_from_cstr("Goodbye Sailor");
  
  stban_sb_chop_by_substr(&left, &right, "bye", 0,3);
  
  stban_sb_append_null(&left);
  stban_sb_append_null(&right);

  left = stban_sb_read_file("./stban_stringutils.h");

  printf( STB_STR_FMT "\n", STB_STR_ARG(left));
  return 0;
}

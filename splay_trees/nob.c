#define NOB_IMPLEMENTATION
#include "nob.h"

void build(void) {
  Nob_Cmd cmd = {0};
  nob_cmd_append(&cmd, "gcc");
  nob_cmd_append(&cmd, "-g");
  nob_cmd_append(&cmd, "-std=c99");
  nob_cmd_append(&cmd, "-pedantic");
  /* nob_cmd_append(&cmd, "-fsanitize=address"); */
  nob_cmd_append(&cmd, "-o main");
  nob_cmd_append(&cmd, "test.c");
  if (!nob_cmd_run_sync(cmd)) exit(1);
}

void run(void) {
  Nob_Cmd cmd = {0};
  nob_cmd_append(&cmd, "gdb");
  nob_cmd_append(&cmd, "-ex=run");
  nob_cmd_append(&cmd, "-ex=bt");
  nob_cmd_append(&cmd, "-ex=quit");
  nob_cmd_append(&cmd, "--args");
  
  nob_cmd_append(&cmd, "./main.exe");
  nob_cmd_append(&cmd, "hello");
  nob_cmd_append(&cmd, "world");
  if (!nob_cmd_run_sync(cmd)) exit(1);  
}

int main(int argc, char **argv) {
  NOB_GO_REBUILD_URSELF(argc, argv);
  build();
  run();
}

#define NOB_IMPLEMENTATION
#include <nob.h>
#include <string.h>

#define BUILD_DIR "build/"

const char *INPUTS[] = {"src/main.c", "src/parser.c", "src/util.c"};
const size_t INPUTS_COUNT = sizeof(INPUTS) / sizeof(char *);
const char *OUTPUT = BUILD_DIR "tromp";

void cc(Nob_Cmd *cmd) {
  nob_cmd_append(cmd, "clang");
}

void cflags(Nob_Cmd *cmd, bool debug) {
  nob_cmd_append(cmd, "-Wall", "-Wextra", "-Wpedantic");
  nob_cmd_append(cmd, "-Iinclude");

  if (debug) {
    nob_cmd_append(cmd, "-O0");
    nob_cmd_append(cmd, "-ggdb");
  } else {
    nob_cmd_append(cmd, "-O2");
  }
}

void libs(Nob_Cmd *cmd) {
  nob_cmd_append(cmd, "-lm");
  nob_cmd_append(cmd, "-lraylib");
}

typedef struct {
  bool force;
  bool run;
  bool bear;
  bool debug;
} Cli_Args;

Cli_Args parse_args(int argc, char **argv) {
  Cli_Args args = {0};

  for (size_t i = 0; i < (size_t)argc; ++i) {
    char *arg = argv[i];
    args.bear = args.bear || strcmp(arg, "bear") == 0;
    args.force = args.force || strcmp(arg, "force") == 0 || strcmp(arg, "f") == 0;
    args.run = args.run || strcmp(arg, "run") == 0;
    args.debug = args.debug || strcmp(arg, "debug") == 0;
  }

  return args;
}

int main(int argc, char **argv) {
  NOB_GO_REBUILD_URSELF(argc, argv);
  NOB_UNUSED(nob_shift(argv, argc));

  Cli_Args args = parse_args(argc, argv);

  if (!nob_file_exists("build")) {
    Nob_Cmd cmd = {0};
    nob_cmd_append(&cmd, "mkdir", "-p", BUILD_DIR);
    if (!nob_cmd_run_sync(cmd)) {
      nob_log(NOB_ERROR, "Could not create build dir.");
      return 1;
    }
  }

  Nob_Cmd cmd = {0};
  if (args.bear) nob_cmd_append(&cmd, "bear", "--");
  cc(&cmd);
  cflags(&cmd, args.debug);
  libs(&cmd);
  nob_cmd_append(&cmd, "-o", OUTPUT);
  nob_da_append_many(&cmd, INPUTS, INPUTS_COUNT);

  if (args.force || nob_needs_rebuild(OUTPUT, INPUTS, INPUTS_COUNT)) {
    if (!nob_cmd_run_sync(cmd)) return 1;
  }

  if (args.run) {
    Nob_Cmd out_cmd = {0};
    nob_cmd_append(&out_cmd, OUTPUT);
    if (!nob_cmd_run_sync(out_cmd)) return 1;
  }

  return 0;
}

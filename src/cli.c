#define __GNU_SOURCE
#include "container/cli.h"
#include "container/config.h"
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static inline int oputs(char const *str) {
  return fputs(str, stdout);
}

struct option run_opts_long[] = {
  {"name", required_argument, NULL, 'n'},
  {"hostname", required_argument, NULL, 'h'},
  {"rootfs", required_argument, NULL, 'r'},
  {"memory", required_argument, NULL, 'm'},
  {"pids", required_argument, NULL, 'p'},
  {"cpus", required_argument, NULL, 'c'},
  {"cpu_weight", required_argument, NULL, 'w'},
  {"rm", no_argument, NULL, 0},
  {"help", no_argument, NULL, 0},
  {NULL, 0, NULL, 0}
};

static int;

int cli_help(const char *help) {
  printf(
    _("\
Usage: %s [OPTION]... [-T] SOURCE DEST\n\
  or:  %s [OPTION]... SOURCE... DIRECTORY\n\
  or:  %s [OPTION]... -t DIRECTORY SOURCE...\n\
"),
    program_name,
    program_name,
    program_name
  );
  return 0;
}

int cli_run(int argc, char **argv) {
  return 0;
}

int cli_parse_run(int argc, char **argv, struct run_opts *opts) {
  opts->memory = CGROUP_DEFAULT_MEMORY;
  opts->cpu_weights = CGROUP_DEFAULT_CPU_WEIGHT;
  opts->cpus = CGROUP_DEFAULT_CPU_MAX;
  opts->pid = CGROUP_DEFAULT_PIDS;

  int opt = 0;
  int idx = 0;

  optind = 1;
  opterr = 0;

  while ((opt = getopt_long(argc, argv, "", run_opts_long, &idx)) != -1) {
    switch (opt) {
      case 'n':
        opts->name = optarg;
        break;
      case 'h':
        opts->hostname = optarg;
        break;
      case 'r':
        opts->rootfs = optarg;
        break;
      case 'm':
        opts->memory = optarg;
        break;
      case 'p':
        opts->pid = optarg;
        break;
      case 'c':
        opts->cpus = optarg;
        break;
      case 'w':
        opts->cpu_weights = optarg;
        break;
      case 0: {
        if (strcmp(run_opts_long[idx].name, "rm") == 0) {
          opts->remove = true;
        }

        if (strcmp(run_opts_long[idx].name, "help") == 0) {
          return 1;
        }
      }
      default:
        exit(-1);
    }
  }

  return 0;
}

#ifndef CONTAINER_CLI_H
#define CONTAINER_CLI_H

#include <stdbool.h>

struct run_opts {
  const char *name;
  const char *hostname;
  const char *pids;
  const char *rootfs;
  const char *memory;
  const char *cpu_weight;
  const char *cpus;
  bool remove;
  char **argv;
  int argc;
};

void usage(int status);
int cli_run(int argc, char **argv);
int cli_parse_run(int argc, char **argv, struct run_opts *opts);

#endif

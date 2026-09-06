#define __GNU_SOURCE
#include "container/cli.h"
#include "container/cgroups.h"
#include "container/config.h"
#include "container/namespaces.h"
#include "log.h"
#include <getopt.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define PROGRAM_NAME "container"
#define _(msgid) (msgid)

static inline int oputs(char const *str) {
  return fputs(str, stdout);
}

struct option run_opts_long[] = {
  {"name", required_argument, 0, 'n'},
  {"hostname", required_argument, 0, 'h'},
  {"rootfs", required_argument, 0, 'r'},
  {"memory", required_argument, 0, 'm'},
  {"pids", required_argument, 0, 'p'},
  {"cpus", required_argument, 0, 'c'},
  {"cpu_weight", required_argument, 0, 'w'},
  {"rm", no_argument, 0, 0},
  {"help", no_argument, 0, 0},
  {0, 0, 0, 0}
};

void usage(int status) {
  if (status != EXIT_SUCCESS) {
    fprintf(stderr, _("Try '%s --help' for more information.\n"), PROGRAM_NAME);
  } else {
    printf(
      _("\
Usage: %s run [OPTIONS] <rootfs> <command> [args...]\n\
  or:  %s run --rootfs PATH [OPTIONS] <command> [args...]\n\
"
        "\n"),
      PROGRAM_NAME,
      PROGRAM_NAME
    );
    fputs(
      _("\
Create and run a new container.\n\
\n\
"),
      stdout
    );
    oputs(_("\
  -n, --name NAME         Container name (cgroup id)\n\
"));
    oputs(_("\
  -h, --hostname HOST     Hostname inside container\n\
"));
    oputs(_("\
  -r, --rootfs PATH       Root filesystem path, can be anywhere on host\n\
"));
    oputs(_("\
  -m, --memory 1G         Memory limit (e.g. 1G, 512M, max)\n\
"));
    oputs(_("\
  -p, --pids N            Max number of processes\n\
"));
    oputs(_("\
  -c, --cpus 0.5          CPU allocation (0.5 = 50000 100000)\n\
"));
    oputs(_("\
      --cpu-weight 100    CPU weight 1..10000\n\
"));
    oputs(_("\
      --rm                Remove cgroup after exit\n\
"));
    oputs(_("\
      --help              Display this help and exit\n\
"));
    fputs(
      _("\
\n\
Examples:\n\
"),
      stdout
    );
    printf(
      _("\
  %s run --name web --memory 1G ./rootfs /bin/sh\n\
"),
      PROGRAM_NAME
    );
    printf(
      _("\
  %s run --rootfs /tmp/alpine-rootfs --hostname box /bin/sh -c 'echo $$'\n\
"),
      PROGRAM_NAME
    );
  };
}

int cli_parse_run(int argc, char **argv, struct run_opts *opts) {
  memset(opts, 0, sizeof(*opts));
  opts->memory = CGROUP_DEFAULT_MEMORY;
  opts->cpu_weight = CGROUP_DEFAULT_CPU_WEIGHT;
  opts->cpus = CGROUP_DEFAULT_CPU_MAX;
  opts->pids = CGROUP_DEFAULT_PIDS;

  int opt = 0;
  int idx = 0;

  optind = 1;
  opterr = 0;

  while ((opt = getopt_long(argc, argv, "n:h:r:m:p:c:w:", run_opts_long, &idx)) != -1) {
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
        opts->pids = optarg;
        break;
      case 'c':
        opts->cpus = optarg;
        break;
      case 'w':
        opts->cpu_weight = optarg;
        break;
      case 0: {
        if (strcmp(run_opts_long[idx].name, "rm") == 0) {
          opts->remove = true;
        } else if (strcmp(run_opts_long[idx].name, "help") == 0) {
          usage(EXIT_SUCCESS);
        }
        break;
      }
      default:
        LOG_ERROR("Unknown option: %s", argv[optind - 1]);
        return -1;
    }
  }

  if (opts->rootfs == NULL && optind < argc)
    opts->rootfs = argv[optind++];
  if (opts->rootfs == NULL) {
    LOG_ERROR("rootfs is required");
    return -1;
  }
  if (optind >= argc) {
    LOG_ERROR("Command is required: %s <rootfs> <command> [args...]", PROGRAM_NAME);
    return -1;
  }
  opts->argv = &argv[optind];
  opts->argc = argc - optind;

  return 0;
}

int cli_run(int argc, char **argv) {
  if (argc < 2) {
    usage(EXIT_FAILURE);
    return EXIT_FAILURE;
  }

  if (argc >= 2 && (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "help") == 0)) {
    usage(EXIT_SUCCESS);
    return EXIT_SUCCESS;
  }

  if (strcmp(argv[1], "run") == 0) {
    struct run_opts opts;
    if (cli_parse_run(argc - 1, argv + 1, &opts) == -1) {
      usage(EXIT_FAILURE);
      return EXIT_FAILURE;
    }

    if (
      argc >= 2 &&
      (strcmp(argv[2], "--help") == 0 || strcmp(argv[2], "help") == 0 || strcmp(argv[2], "-h") == 0)
    ) {
      usage(EXIT_SUCCESS);
      return EXIT_SUCCESS;
    }

    int status;
    pid_t pid;

    cgroup_enable_controllers();
    cgroup_create(opts.name);
    container_spawn(&pid, opts.argv, opts.hostname, opts.rootfs);
    cgroup_add_proc(opts.name, pid);
    cgroup_set(opts.name, "pids.max", opts.pids);
    cgroup_set(opts.name, "memory.max", opts.memory);
    waitpid(pid, &status, 0);
    if (opts.remove) {
      cgroup_destroy(opts.name);
    }
  }
  return 0;
}

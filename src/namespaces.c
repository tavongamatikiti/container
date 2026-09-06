#define _GNU_SOURCE
#include "container/namespaces.h"
#include "container/mount.h"
#include "log.h"
#include <errno.h>
#include <limits.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#ifdef __linux__
#include <sched.h>
#include <signal.h>
#include <sys/mount.h>
#else
#include <signal.h>
#endif

#define STACK_SIZE (1024 * 1024)

#ifdef __linux__
static char child_stack[STACK_SIZE];

struct child_arg {
  char *const *argv;
  const char *hostname;
  const char *rootfs;
};

static int child_fn(void *arg) {
  struct child_arg *val = arg;
  if (val->hostname != NULL) {
    size_t len = strlen(val->hostname);
#ifdef __linux__
    if (sethostname(val->hostname, len) == -1)
      LOG_ERROR("Failed to set hostname: %s", strerror(errno));
#else
    (void)len;
#endif
  }

  if (val->rootfs != NULL) {
    if (set_mount(val->rootfs, "oldroot") == -1) {
      LOG_ERROR("Failed to setup container rootfs: %s", strerror(errno));
      return 1;
    }
  }

  char command_path[PATH_MAX];
  if (val->argv[0][0] == '/') {
    snprintf(command_path, sizeof(command_path), "%s", val->argv[0]);
  } else if (strchr(val->argv[0], '/') != NULL) {
    snprintf(command_path, sizeof(command_path), "/%s", val->argv[0]);
  } else {
    snprintf(command_path, sizeof(command_path), "/bin/%s", val->argv[0]);
  }

  execv(command_path, (char *const *)val->argv);
  LOG_ERROR("Failed to execute %s: %s", val->argv[0], strerror(errno));
  return 1;
}
#endif

int container_spawn(pid_t *pid, char *const argv[], const char *hostname, const char *rootfs) {
#ifdef __linux__

  struct child_arg arg = {argv, hostname, rootfs};

  int flags = CLONE_NEWPID | CLONE_NEWNS | CLONE_NEWUTS | CLONE_NEWIPC | CLONE_NEWCGROUP | SIGCHLD;

  *pid = clone(child_fn, child_stack + STACK_SIZE, flags, &arg);

  if (*pid == -1) {
    LOG_ERROR("Failed to clone: %s", strerror(errno));
    return -1;
  }

  return 0;

#else

  (void)pid;
  (void)argv;
  (void)hostname;
  (void)rootfs;
  errno = ENOSYS;

  return -1;

#endif
}

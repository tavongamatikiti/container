#define _GNU_SOURCE
#include "container/namespaces.h"
#include <errno.h>
#include <limits.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#ifdef __linux__
#include "container/mount.h"
#include "log.h"
#include <arpa/inet.h>
#include <net/if.h>
#include <sched.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <sys/mount.h>
#include <sys/socket.h>
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

  int fds = socket(AF_INET, SOCK_DGRAM, 0);
  if (fds == -1) {
    LOG_ERROR("Failed to create network socket: %s", strerror(errno));
    return -1;
  }

  struct ifreq ifr = {0};
  strncpy(ifr.ifr_name, "lo", IFNAMSIZ - 1);

  if (ioctl(fds, SIOCGIFFLAGS, &ifr)) {
    LOG_ERROR("Failed to get flags for interface '%s': %s", ifr.ifr_name, strerror(errno));
    close(fds);
    return -1;
  }

  ifr.ifr_flags |= IFF_UP;

  if (ioctl(fds, SIOCSIFFLAGS, &ifr) == -1) {
    LOG_ERROR("Failed to bring interface '%s' up: %s", ifr.ifr_name, strerror(errno));
    close(fds);
    return -1;
  }

  close(fds);

  LOG_DEBUG("Loopback interface '%s' is up", ifr.ifr_name);

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

  int flags = CLONE_NEWPID | CLONE_NEWNS | CLONE_NEWUTS | CLONE_NEWIPC | CLONE_NEWCGROUP |
    CLONE_NEWNET | SIGCHLD;

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

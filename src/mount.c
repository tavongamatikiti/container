#define _GNU_SOURCE
#include "container/mount.h"
#include "log.h"

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#ifdef __linux__
#include "container/config.h"
#include <sys/mount.h>
#include <sys/syscall.h>
#endif

#define DEFAULT_OLDROOT "oldroot"

#ifdef __linux__
static long pivot_root_syscall(const char *new_root, const char *old_root) {
  return syscall(SYS_pivot_root, new_root, old_root);
}
#endif

int set_mount(const char *rootfs, const char *oldroot_name) {
#ifdef __linux__
  if (rootfs == NULL) {
    LOG_ERROR("Rootfs path is required");
    errno = EINVAL;
    return -1;
  }

  if (oldroot_name == NULL) {
    oldroot_name = DEFAULT_OLDROOT;
  }

  char oldroot_path[PATH_MAX];
  int path_len = snprintf(oldroot_path, sizeof(oldroot_path), "%s/%s", rootfs, oldroot_name);
  if (path_len < 0 || path_len >= (int)sizeof(oldroot_path)) {
    LOG_ERROR("Rootfs oldroot path too long: %s/%s", rootfs, oldroot_name);
    errno = ENAMETOOLONG;
    return -1;
  }

  LOG_INFO("Mount setup for rootfs %s with oldroot %s", rootfs, oldroot_name);
  LOG_DEBUG("Oldroot full path %s", oldroot_path);

  LOG_DEBUG("Making mount tree private");
  if (mount(NULL, "/", NULL, MS_REC | MS_PRIVATE, NULL) == -1) {
    LOG_ERROR("Failed to make mount tree private: %s", strerror(errno));
    return -1;
  }
  LOG_INFO("Mount tree is private");

  LOG_DEBUG("Bind mounting rootfs %s", rootfs);
  if (mount(rootfs, rootfs, NULL, MS_BIND | MS_REC, NULL) == -1) {
    LOG_ERROR("Failed to bind mount rootfs %s: %s", rootfs, strerror(errno));
    return -1;
  }
  LOG_INFO("Bind mounted rootfs %s", rootfs);

  if (mkdir(oldroot_path, 0755) == -1 && errno != EEXIST) {
    LOG_ERROR("Failed to create oldroot directory %s: %s", oldroot_path, strerror(errno));
    return -1;
  }
  LOG_DEBUG("Created oldroot directory %s", oldroot_path);

  LOG_INFO("Pivoting root to %s with oldroot %s", rootfs, oldroot_path);
  if (pivot_root_syscall(rootfs, oldroot_path) == -1) {
    LOG_ERROR(
      "Failed to pivot root to %s with oldroot %s: %s", rootfs, oldroot_path, strerror(errno)
    );
    return -1;
  }

  if (chdir("/") == -1) {
    LOG_ERROR("Failed to change directory to /: %s", strerror(errno));
    return -1;
  }
  LOG_DEBUG("Changed directory to /");

  char oldroot_mount[PATH_MAX];
  path_len = snprintf(oldroot_mount, sizeof(oldroot_mount), "/%s", oldroot_name);
  if (path_len < 0 || path_len >= (int)sizeof(oldroot_mount)) {
    LOG_ERROR("Oldroot mount path too long for %s", oldroot_name);
    errno = ENAMETOOLONG;
    return -1;
  }
  LOG_DEBUG("Oldroot mount path %s", oldroot_mount);

  if (umount2(oldroot_mount, MNT_DETACH) == -1) {
    LOG_ERROR("Failed to unmount oldroot %s: %s", oldroot_mount, strerror(errno));
    return -1;
  }
  LOG_DEBUG("Unmounted oldroot %s", oldroot_mount);

  if (rmdir(oldroot_mount) == -1) {
    LOG_ERROR("Failed to remove oldroot directory %s: %s", oldroot_mount, strerror(errno));
    return -1;
  }
  LOG_DEBUG("Removed oldroot directory %s", oldroot_mount);

  if (mkdir("/proc", 0555) == -1 && errno != EEXIST) {
    LOG_ERROR("Failed to create /proc: %s", strerror(errno));
    return -1;
  }

  LOG_DEBUG("Mounting proc at /proc");
  if (mount("proc", "/proc", "proc", 0, NULL) == -1) {
    LOG_ERROR("Failed to mount /proc: %s", strerror(errno));
    return -1;
  }
  LOG_INFO("Mounted /proc for container");

  if (mkdir("/sys", 0555) == -1 && errno != EEXIST) {
    LOG_ERROR("Failed to create /sys folder: %s", strerror(errno));
    return -1;
  }

  if (mkdir("/sys/fs", 0555) == -1 && errno != EEXIST) {
    LOG_ERROR("Failed to create /sys/fs folder: %s", CGROUP_ROOT, strerror(errno));
    return -1;
  }

  if (mkdir(CGROUP_ROOT, 0755) == -1 && errno != EEXIST) {
    LOG_ERROR("Failed to create %s cgroup: %s", CGROUP_ROOT, strerror(errno));
    return -1;
  }

  LOG_DEBUG("Mounting cgroup2 at /sys/fs/cgroup");

  if (mount("cgroup2", CGROUP_ROOT, "cgroup2", 0, NULL) == -1) {
    LOG_ERROR("Failed to mount %s: %s", CGROUP_ROOT, strerror(errno));
    return -1;
  }

  LOG_INFO("Mounted cgroup namespace at /sys/fs/cgroup");

  return 0;
#else
  (void)rootfs;
  (void)oldroot_name;
  LOG_ERROR("Mount namespaces are only supported on Linux");
  errno = ENOSYS;
  return -1;
#endif
}

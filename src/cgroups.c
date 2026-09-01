#define _GNU_SOURCE
#include "container/cgroups.h"
#include "log.h"

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define CGROUP_ROOT "/sys/fs/cgroup"
#define CGROUP_PROCS "cgroup.procs"
#define CGROUP_CONTROLLERS "cgroup.subtree_control"

int cgroup_create(const char *name) {
  char cgroup_path[PATH_MAX];

  if (name == NULL) {
    LOG_ERROR("Cgroup name is required");
    errno = EINVAL;
    return -1;
  }

  int path_len = snprintf(cgroup_path, sizeof(cgroup_path), "%s/%s", CGROUP_ROOT, name);
  if (path_len < 0 || path_len >= (int)sizeof(cgroup_path)) {
    LOG_ERROR("Cgroup path too long for %s/%s", CGROUP_ROOT, name);
    errno = ENAMETOOLONG;
    return -1;
  }

  LOG_DEBUG("Creating cgroup %s", cgroup_path);
  if (mkdir(cgroup_path, 0755) == -1 && errno != EEXIST) {
    LOG_ERROR("Failed to create cgroup %s: %s", cgroup_path, strerror(errno));
    return -1;
  }

  if (errno == EEXIST) {
    LOG_WARN("Cgroup %s already exists", cgroup_path);
  } else {
    LOG_INFO("Created cgroup %s", cgroup_path);
  }

  return 0;
}

int cgroup_set(const char *cgroup, const char *key, const char *value) {
  char control_path[PATH_MAX];

  if (cgroup == NULL || key == NULL || value == NULL) {
    LOG_ERROR("Cgroup, key and value are required for cgroup_set");
    errno = EINVAL;
    return -1;
  }

  int path_len = snprintf(control_path, sizeof(control_path), "%s/%s/%s", CGROUP_ROOT, cgroup, key);
  if (path_len < 0 || path_len >= (int)sizeof(control_path)) {
    LOG_ERROR("Cgroup control path too long: %s/%s/%s", CGROUP_ROOT, cgroup, key);
    errno = ENAMETOOLONG;
    return -1;
  }

  LOG_DEBUG("Writing %s to %s", value, control_path);
  int fds = open(control_path, O_WRONLY);
  if (fds == -1) {
    LOG_ERROR("Failed to open cgroup file %s: %s", control_path, strerror(errno));
    return -1;
  }

  size_t value_len = strlen(value);
  ssize_t bytes_written = write(fds, value, value_len);
  if (bytes_written != (ssize_t)value_len) {
    LOG_ERROR("Failed to write %s to %s: %s", value, control_path, strerror(errno));
    close(fds);
    return -1;
  }

  if (close(fds) == -1) {
    LOG_ERROR("Failed to close cgroup file %s: %s", control_path, strerror(errno));
    return -1;
  }

  LOG_INFO("Set %s=%s for cgroup %s", key, value, cgroup);
  return 0;
}

int cgroup_add_proc(const char *cgroup, pid_t pid) {
  char procs_path[PATH_MAX];
  char pid_text[32];

  if (cgroup == NULL) {
    LOG_ERROR("Cgroup name is required for cgroup_add_proc");
    errno = EINVAL;
    return -1;
  }

  int path_len =
      snprintf(procs_path, sizeof(procs_path), "%s/%s/%s", CGROUP_ROOT, cgroup, CGROUP_PROCS);
  if (path_len < 0 || path_len >= (int)sizeof(procs_path)) {
    LOG_ERROR("Cgroup procs path too long: %s/%s/%s", CGROUP_ROOT, cgroup, CGROUP_PROCS);
    errno = ENAMETOOLONG;
    return -1;
  }

  int text_len = snprintf(pid_text, sizeof(pid_text), "%d", pid);
  if (text_len < 0 || text_len >= (int)sizeof(pid_text)) {
    LOG_ERROR("PID text too long for %d", pid);
    errno = EINVAL;
    return -1;
  }

  LOG_DEBUG("Adding pid %s to %s", pid_text, procs_path);
  int fds = open(procs_path, O_WRONLY);
  if (fds == -1) {
    LOG_ERROR("Failed to open cgroup procs %s: %s", procs_path, strerror(errno));
    return -1;
  }

  size_t pid_len = strlen(pid_text);
  ssize_t bytes_written = write(fds, pid_text, pid_len);
  if (bytes_written != (ssize_t)pid_len) {
    LOG_ERROR("Failed to add pid %s to %s: %s", pid_text, procs_path, strerror(errno));
    close(fds);
    return -1;
  }

  if (close(fds) == -1) {
    LOG_ERROR("Failed to close cgroup procs %s: %s", procs_path, strerror(errno));
    return -1;
  }

  LOG_INFO("Added pid %s to cgroup %s", pid_text, cgroup);
  return 0;
}

int cgroup_destroy(const char *cgroup) {
  char cgroup_path[PATH_MAX];

  if (cgroup == NULL) {
    LOG_ERROR("Cgroup name is required for destroy");
    errno = EINVAL;
    return -1;
  }

  int path_len = snprintf(cgroup_path, sizeof(cgroup_path), "%s/%s", CGROUP_ROOT, cgroup);
  if (path_len < 0 || path_len >= (int)sizeof(cgroup_path)) {
    LOG_ERROR("Cgroup path too long for destroy: %s/%s", CGROUP_ROOT, cgroup);
    errno = ENAMETOOLONG;
    return -1;
  }

  LOG_DEBUG("Removing cgroup %s", cgroup_path);
  if (rmdir(cgroup_path) == -1) {
    LOG_ERROR("Failed to remove cgroup %s: %s", cgroup_path, strerror(errno));
    return -1;
  }

  LOG_INFO("Removed cgroup %s", cgroup_path);
  return 0;
}

int cgroup_enable_controllers(void) {
  char controllers_path[PATH_MAX];
  const char *controllers = "+cpu +memory +pids";

  int path_len = snprintf(controllers_path, sizeof(controllers_path), "%s/%s", CGROUP_ROOT,
                          CGROUP_CONTROLLERS);
  if (path_len < 0 || path_len >= (int)sizeof(controllers_path)) {
    LOG_ERROR("Cgroup controllers path too long: %s/%s", CGROUP_ROOT, CGROUP_CONTROLLERS);
    errno = ENAMETOOLONG;
    return -1;
  }

  LOG_DEBUG("Enabling controllers %s at %s", controllers, controllers_path);
  int fds = open(controllers_path, O_WRONLY);
  if (fds == -1) {
    LOG_ERROR("Failed to open controllers file %s: %s", controllers_path, strerror(errno));
    return -1;
  }

  size_t controllers_len = strlen(controllers);
  ssize_t bytes_written = write(fds, controllers, controllers_len);
  if (bytes_written != (ssize_t)controllers_len) {
    LOG_ERROR("Failed to enable controllers %s: %s", controllers, strerror(errno));
    close(fds);
    return -1;
  }

  if (close(fds) == -1) {
    LOG_ERROR("Failed to close controllers file %s: %s", controllers_path, strerror(errno));
    return -1;
  }

  LOG_INFO("Enabled cgroup controllers %s", controllers);
  return 0;
}

#ifndef CONTAINER_CGROUPS_H
#define CONTAINER_CGROUPS_H
#include <sys/types.h>

// cgroup v2 API – implemented in src/cgroups.c
int cgroup_create(const char *name);
int cgroup_set(const char *cgroup, const char *key, const char *value);
int cgroup_add_proc(const char *cgroup, pid_t pid);
int cgroup_destroy(const char *cgroup);
int cgroup_enable_controllers(void);

#endif

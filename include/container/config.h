
#ifndef CONTAINER_CONFIG_H
#define CONTAINER_CONFIG_H

#define CGROUP_ROOT "/sys/fs/cgroup"
#define CGROUP_CONTROLLERS_FILE "cgroup.subtree_control"
#define CGROUP_ENABLE_CONTROLLERS "+cpu +memory +pids"

#define CGROUP_DEFAULT_MEMORY "max"
#define CGROUP_DEFAULT_PIDS "max"
#define CGROUP_DEFAULT_CPU_MAX "max 100000"
#define CGROUP_DEFAULT_CPU_WEIGHT "100"

#define CONTAINER_NAME_MAX 64
#define CONTAINER_NAME_PREFIX "container-"

#endif

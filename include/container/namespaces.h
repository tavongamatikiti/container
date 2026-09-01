#ifndef CONTAINER_NAMESPACES_H
#define CONTAINER_NAMESPACES_H

#include <sys/types.h>
#ifdef __linux__
#include <sched.h>
#endif

// Create an isolated process to run a container.
int container_spawn(pid_t *pid, char *const argv[], const char *hostname, const char *rootfs);

#endif /* CONTAINER_NAMESPACES_H */

#define _GNU_SOURCE
#include "container/container.h"
#include "container/cli.h"
#include "container/version.h"
#include "log.h"
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "container/namespaces.h"

int container_add(int a, int b) {
  return a + b;
}
int container_version_major(void) {
  return CONTAINER_VERSION_MAJOR;
}
const char *container_version(void) {
  return CONTAINER_VERSION_STRING;
}
size_t container_strlen(const char *s) {
  if (s == NULL)
    return 0;
  return strlen(s);
}

#ifndef CONTAINER_LIB
int main(int argc, char *argv[]) {
  return cli_run(argc, argv);
}

#endif

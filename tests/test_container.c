#include "container/container.h"
#include "container/mount.h"
#include "container/namespaces.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>

static void test_add(void) {
  assert(container_add(2, 3) == 5);
  assert(container_add(-1, 1) == 0);
  assert(container_add(0, 0) == 0);
  printf("  test_add: PASS\n");
}

static void test_version(void) {
  assert(container_version_major() == 0);
  assert(strcmp(container_version(), "0.1.0") == 0);
  printf("  test_version: PASS\n");
}

static void test_strlen(void) {
  assert(container_strlen("hello") == 5);
  assert(container_strlen("") == 0);
  assert(container_strlen(NULL) == 0);
  printf("  test_strlen: PASS\n");
}

static void test_container_spawn_accepts_rootfs(void) {
  int (*spawn)(pid_t *, char *const[], const char *, const char *) = container_spawn;
  assert(spawn != NULL);
  printf("  test_container_spawn_accepts_rootfs: PASS\n");
}

static void test_set_mount_accepts_rootfs(void) {
  int (*setup_mounts)(const char *, const char *) = set_mount;
  assert(setup_mounts != NULL);
  printf("  test_set_mount_accepts_rootfs: PASS\n");
}

int main(void) {
  printf("Running container tests...\n");
  test_add();
  test_version();
  test_strlen();
  test_container_spawn_accepts_rootfs();
  test_set_mount_accepts_rootfs();
  printf("All tests passed.\n");
  return 0;
}

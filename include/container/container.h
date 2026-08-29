#ifndef CONTAINER_CONTAINER_H
#define CONTAINER_CONTAINER_H

#include <stddef.h>

/**
 * @brief container – example lib following Luca Vallin flat layout.
 *
 * Each public module has a header in include/container/<name>.h
 * and implementation in src/<name>.c
 */

int container_add(int a, int b);
int container_version_major(void);
const char *container_version(void);

// Example: string utility
size_t container_strlen(const char *s);

#endif /* CONTAINER_CONTAINER_H */

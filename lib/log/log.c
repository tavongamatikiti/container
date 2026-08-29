#include "log.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#define COLOR_RESET "\x1b[0m"
#define COLOR_RED "\x1b[31m"
#define COLOR_GREEN "\x1b[32m"
#define COLOR_YELLOW "\x1b[33m"
#define COLOR_CYAN "\x1b[36m"
#define COLOR_MAGENTA "\x1b[35m"
#define COLOR_BOLD_RED "\x1b[1;31m"

static const char *log_color(const char *level) {
  if (strcmp(level, "INFO") == 0)
    return COLOR_GREEN;
  if (strcmp(level, "DEBUG") == 0)
    return COLOR_CYAN;
  if (strcmp(level, "WARN") == 0)
    return COLOR_YELLOW;
  if (strcmp(level, "ERROR") == 0)
    return COLOR_RED;
  if (strcmp(level, "FATAL") == 0)
    return COLOR_BOLD_RED;
  return COLOR_MAGENTA;
}

static const char *short_file(const char *file) {
  const char *src = strstr(file, "/src/");
  if (src != NULL)
    return src + 1;
  const char *lib = strstr(file, "/lib/");
  if (lib != NULL)
    return lib + 1;
  const char *inc = strstr(file, "/include/");
  if (inc != NULL)
    return inc + 1;
  return file;
}

void log_write(const char *level, const char *file, int line, const char *format, ...) {
  va_list args;

  char time_text[9];
  time_t now = time(NULL);
  struct tm local_time;
  localtime_r(&now, &local_time);
  strftime(time_text, sizeof(time_text), "%H:%M:%S", &local_time);

  const char *color = log_color(level);
  const char *display_file = short_file(file);

  if (strcmp(level, "DEBUG") == 0) {
    printf("%s %s%s%s %s:%d ", time_text, color, level, COLOR_RESET, display_file, line);
  } else {
    printf("%s %s%s%s %s ", time_text, color, level, COLOR_RESET, display_file);
  }

  va_start(args, format);
  vprintf(format, args);
  va_end(args);

  printf("\n");
}

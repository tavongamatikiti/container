#ifndef LOG_H
#define LOG_H

void log_write(const char *level, const char *file, int line, const char *format, ...);

#define LOG_INFO(...) log_write("INFO", __FILE__, __LINE__, __VA_ARGS__)
#define LOG_ERROR(...) log_write("ERROR", __FILE__, __LINE__, __VA_ARGS__)
#define LOG_WARN(...) log_write("WARN", __FILE__, __LINE__, __VA_ARGS__)
#define LOG_FATAL(...) log_write("FATAL", __FILE__, __LINE__, __VA_ARGS__)
#define LOG_DEBUG(...) log_write("DEBUG", __FILE__, __LINE__, __VA_ARGS__)

#endif

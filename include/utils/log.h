#ifndef INCLUDE_UTILS_LOG_H_
#define INCLUDE_UTILS_LOG_H_

#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <pthread.h>

typedef enum {
  LOG_INFO,
  LOG_WARNING,
  LOG_ERROR
} LogLevel;

void log_open(void);
void log_close(void);
void log_msg(const char *module, LogLevel level, const char *fmt, ...);

void test_log_open(void);
void test_log_close(void);
void test_log_msg(const char *module, LogLevel level, const char *fmt, ...);

#define LOG_INFO(module, ...) log_msg(module, LOG_INFO, __VA_ARGS__)
#define LOG_WARN(module, ...) log_msg(module, LOG_WARNING, __VA_ARGS__)
#define LOG_ERROR(module, ...) log_msg(module, LOG_ERROR, __VA_ARGS__)

#endif  // INCLUDE_UTILS_LOG_H_

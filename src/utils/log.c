#include "../../include/utils/log.h"
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <errno.h>

static FILE *log_fp = NULL;
static FILE *test_log_fp = NULL;
static pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;

static const char *level_str(LogLevel level) {
  switch (level) {
    case LOG_INFO:    return "INFO";
    case LOG_WARNING: return "WARNING";
    case LOG_ERROR:   return "ERROR";
    default:          return "UNKNOWN";
  }
}

static void write_timestamp(FILE *fp) {
  time_t now = time(NULL);
  struct tm *t = localtime(&now);
  fprintf(fp, "[%04d-%02d-%02d %02d:%02d:%02d] ",
          t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
          t->tm_hour, t->tm_min, t->tm_sec);
}

void log_open(void) {
  pthread_mutex_lock(&log_mutex);
  if (!log_fp) {
    mkdir("./logs", 0755);
    log_fp = fopen("./logs/zapret2-tui.log", "a");
    if (log_fp) {
      write_timestamp(log_fp);
      fprintf(log_fp, "[main] [INFO]: Application started\n");
      fflush(log_fp);
    }
  }
  test_log_open();
  pthread_mutex_unlock(&log_mutex);
}

void log_close(void) {
  pthread_mutex_lock(&log_mutex);
  if (log_fp) {
    write_timestamp(log_fp);
    fprintf(log_fp, "[main] [INFO]: Application shutdown\n\n");
    fclose(log_fp);
    log_fp = NULL;
  }
  test_log_close();
  pthread_mutex_unlock(&log_mutex);
}

void log_msg(const char *module, LogLevel level, const char *fmt, ...) {
  pthread_mutex_lock(&log_mutex);
  if (log_fp) {
    write_timestamp(log_fp);
    fprintf(log_fp, "[%s] [%s]: ", module, level_str(level));
    va_list args;
    va_start(args, fmt);
    vfprintf(log_fp, fmt, args);
    va_end(args);
    fprintf(log_fp, "\n");
    fflush(log_fp);
  }
  pthread_mutex_unlock(&log_mutex);
}

void test_log_open(void) {
  if (!test_log_fp) {
    mkdir("./logs", 0755);
    test_log_fp = fopen("./logs/testing.log", "a");
    if (test_log_fp) {
      write_timestamp(test_log_fp);
      fprintf(test_log_fp, "=== Testing session started ===\n");
      fflush(test_log_fp);
    }
  }
}

void test_log_close(void) {
  if (test_log_fp) {
    write_timestamp(test_log_fp);
    fprintf(test_log_fp, "=== Testing session ended ===\n\n");
    fclose(test_log_fp);
    test_log_fp = NULL;
  }
}

void test_log_msg(const char *module, LogLevel level, const char *fmt, ...) {
  if (test_log_fp) {
    write_timestamp(test_log_fp);
    fprintf(test_log_fp, "[%s] [%s]: ", module, level_str(level));
    va_list args;
    va_start(args, fmt);
    vfprintf(test_log_fp, fmt, args);
    va_end(args);
    fprintf(test_log_fp, "\n");
    fflush(test_log_fp);
  }
}

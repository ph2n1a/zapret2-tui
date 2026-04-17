#include <stdbool.h>

#ifndef READCONF_H
#define READCONF_H

typedef struct {
  bool first_start;
  char zapret_path[512];
} Config;

int handler(void *user, const char *section, const char *name, const char *value);

#endif

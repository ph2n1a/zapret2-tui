#ifndef READWRITECONF_H
#define READWRITECONF_H

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <confuse.h>

typedef struct {
  bool first_start;
  char zapret_path[512];
} Config;

int read_conf_sec(Config *config_main);

#endif

/* #include <stdbool.h>

#ifndef READCONF_H
#define READCONF_H

typedef struct {
  bool first_start;
  char zapret_path[512];
} Config;

int handler(void *user, const char *section, const char *name, const char *value);

#endif */

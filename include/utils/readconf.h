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
char* read_conf_char(char *name);
bool read_conf_bool(char *name);

#endif

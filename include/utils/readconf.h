#ifndef READWRITECONF_H
#define READWRITECONF_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <confuse.h>

typedef struct {
  bool first_start;
  char zapret_path[512];
  char program_path[512];
  int view_profile;
} Config;

typedef struct {
  int id;
  char name[128];
  char nfqws2_opt[4096];
} Profile;

int read_conf_config(Config *config, const char *section);
Profile* read_conf_profiles(const char *section, short *count, short *error_code);

#endif

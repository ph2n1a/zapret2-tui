#ifndef INCLUDE_CORE_READ_ZAPRET_CONF_H_
#define INCLUDE_CORE_READ_ZAPRET_CONF_H_

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

typedef struct {
  int start;
  int finish;
  short lines;
  char text[256][512];
  bool stand_format;
} ZapretConf;

ZapretConf read_conf_engine(char *zapret_path_n);

#endif  // INCLUDE_CORE_READ_ZAPRET_CONF_H_

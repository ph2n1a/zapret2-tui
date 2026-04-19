#ifndef WRITECONF_H
#define WRITECONF_H

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <confuse.h>

int write_conf_sec(const char *cfg_set, const char *name, const char *value);

#endif // !WRITECONF_H

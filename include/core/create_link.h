#ifndef INCLUDE_CORE_CREATE_LINK_H_
#define INCLUDE_CORE_CREATE_LINK_H_

#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include "../utils/utils.h"
#include "../utils/writeconf.h"

int create_link(char *zapret_path, int *n_profile);

#endif  // INCLUDE_CORE_CREATE_LINK_H_

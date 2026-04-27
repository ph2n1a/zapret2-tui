#ifndef INCLUDE_CORE_CREATE_ZAPRET_CONFIGS_H_
#define INCLUDE_CORE_CREATE_ZAPRET_CONFIGS_H_

#include <stdio.h>
#include <stdbool.h>
#include "./read_zapret_conf.h"
#include "../utils/readconf.h"

int create_zapret_configs(ZapretConf zapret_config, Profile profile[], const short *n_profiles);

#endif  // INCLUDE_CORE_CREATE_ZAPRET_CONFIGS_H_

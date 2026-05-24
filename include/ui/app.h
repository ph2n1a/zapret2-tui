#ifndef INCLUDE_UI_APP_H_
#define INCLUDE_UI_APP_H_

#include <ncurses.h>
#include <stdlib.h>
#include "./core.h"
#include "./input.h"
#include "./ui.h"
#include "../utils/readconf.h"

int  app_init(Profile *profile_get, short *n_profiles_get, Config main_conf_get);
void app_run();
void app_cleanup();

#endif  // INCLUDE_UI_APP_H_

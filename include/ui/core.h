#ifndef INCLUDE_UI_CORE_H_
#define INCLUDE_UI_CORE_H_

#include "../core/create_link.h"
#include "../utils/writeconf.h"
#include "../core/testing_profiles.h"
#include <stdbool.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <pthread.h>

#define APP_MAX_PROFILES 1024

typedef enum {
  INPUT_NONE = 0,
  INPUT_UP,
  INPUT_DOWN,
  INPUT_ENTER,
  INPUT_QUIT,
  INPUT_REINIT,
  INPUT_RELOAD_SERVICE,
  INPUT_START_SERVICE,
  INPUT_STOP_SERVICE,
  INPUT_OPEN_HELP_WINDOW,
  INPUT_TESTING_ONE,
  INPUT_TESTING_ALL,
} InputAction;

typedef struct {
  int menu_index;
  int menu_count;
  int view_profile;
  bool is_reload;
  bool service_error;
  bool help_window;
  bool error_window;
  char error_message[256];
  int running;

  int testing_profiles[APP_MAX_PROFILES];
} AppState;

void core_init(AppState *state, short *n_profiles, Profile *profiles, char *get_zapret_path, bool get_without_sudo, Testing testing_get);
void core_update(AppState *state, InputAction action);
int zapret2_ctl(int code);

#endif  // INCLUDE_UI_CORE_H_

#ifndef INCLUDE_UI_CORE_H_
#define INCLUDE_UI_CORE_H_

#include "../core/create_link.h"
#include "../utils/writeconf.h"
#include <stdbool.h>
#include <stdlib.h>
#include <sys/wait.h>

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
} AppState;

void core_init(AppState *state, short *n_profiles, char *get_zapret_path, bool get_without_sudo);
void core_update(AppState *state, InputAction action);
int zapret2_ctl(int code);

#endif  // INCLUDE_UI_CORE_H_

#include "../../include/ui/app.h"

static AppState g_state;
static Profile *profile;
static Config main_conf;
static short n_profiles;

int app_init(Profile *profile_get, short *n_profiles_get) {
  if (!initscr()) return -1;

  cbreak();
  noecho();
  keypad(stdscr, TRUE);
  curs_set(0);
  timeout(50);

  profile = profile_get;
  n_profiles = *n_profiles_get;
  read_conf_config(&main_conf, "main");

  core_init(&g_state, &n_profiles);
  ui_init();

  return 0;
}

void app_run() {
  while (g_state.running) {
    InputAction action = input_poll();
    core_update(&g_state, action);

    if (action == INPUT_REINIT) {
      clearok(stdscr, TRUE); 
      refresh();
      ui_init();
      ui_draw(&g_state, profile, &n_profiles, &main_conf.view_profile);
      continue;
    }

    ui_draw(&g_state, profile, &n_profiles, &main_conf.view_profile);
    refresh();
  }
}

void app_cleanup() {
  ui_cleanup();
  endwin();
}

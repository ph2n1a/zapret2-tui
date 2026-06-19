#include "../../include/ui/app.h"
#include "../../include/utils/log.h"

static AppState g_state;
static Profile *profile;
static Config main_conf;
static short n_profiles;
static bool without_sudo;
static int reload_timer_count;

void reload_timer(int i) {
  reload_timer_count += i;
  if (reload_timer_count == 22) {
    reload_timer_count = 0;
    g_state.is_reload = false;
  }
}

int app_init(Profile *profile_get, short *n_profiles_get, Config main_conf_get, Testing testing_get) {
  LOG_INFO("app", "Initializing TUI application");
  if (!initscr()) return -1;

  cbreak();
  noecho();
  keypad(stdscr, TRUE);
  curs_set(0);
  start_color();
  timeout(50);

  profile = profile_get;
  n_profiles = *n_profiles_get;
  main_conf = main_conf_get;
  g_state.view_profile = main_conf.view_profile;
  without_sudo = main_conf_get.without_sudo;
  for (int i = 0; i < APP_MAX_PROFILES; i++) {
    g_state.testing_profiles[i] = TESTING_PROFILE_NOT_TESTED;
  }

  core_init(&g_state, &n_profiles, profile, main_conf.zapret_path, without_sudo, testing_get);
  ui_init(without_sudo);

  return 0;
}

void app_run() {
  LOG_INFO("app", "Entering main event loop");
  while (g_state.running) {
    InputAction action = input_poll();
    core_update(&g_state, action);

    if (action == INPUT_REINIT) {
      clearok(stdscr, TRUE); 
      refresh();
      ui_init(without_sudo);
      ui_draw(&g_state, profile, &g_state.view_profile);
      continue;
    }

    ui_draw(&g_state, profile, &g_state.view_profile);
    if (g_state.is_reload) reload_timer(1);
    refresh();
  }
}

void app_cleanup() {
  LOG_INFO("app", "Cleaning up application");
  testing_log_close();
  ui_cleanup();
  endwin();
}

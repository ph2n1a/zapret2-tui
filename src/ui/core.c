#include "../../include/ui/core.h"
#include "../../include/utils/utils.h"
#include <stdio.h>
#include <string.h>

static char *zapret_path;
static bool without_sudo;

static void set_error_window(AppState *state, const char *message) {
    state->error_window = true;
    snprintf(state->error_message, sizeof(state->error_message), "%s", message);
}

static int selected_config_matches_zapret(int profile_id) {
    char user_conf_path[PATH_MAX];
    char zapret_conf_path[PATH_MAX];

    if (snprintf(user_conf_path, sizeof(user_conf_path), "./config/zapret_config/config_%d", profile_id) >= (int)sizeof(user_conf_path)) {
        return -1;
    }
    if (snprintf(zapret_conf_path, sizeof(zapret_conf_path), "%s/config", zapret_path) >= (int)sizeof(zapret_conf_path)) {
        return -1;
    }

    return compare_files(zapret_conf_path, user_conf_path);
}

void core_init(AppState *state, short *n_profiles, char *get_zapret_path, bool get_without_sudo) {
    state->menu_index = 0;
    state->menu_count = *n_profiles;
    state->running = 1;
    state->error_window = false;
    state->error_message[0] = '\0';

    zapret_path = get_zapret_path;
    without_sudo = get_without_sudo;
}

void core_update(AppState *state, InputAction action) {
    if (action == INPUT_QUIT) {
        state->running = 0;
        return;
    }

    if (state->error_window && action != INPUT_NONE) {
        state->error_window = false;
        state->error_message[0] = '\0';
    }

    if (action == INPUT_UP) {
        state->menu_index = (state->menu_index - 1 + state->menu_count) % state->menu_count;
    } else if (action == INPUT_DOWN) {
        state->menu_index = (state->menu_index + 1) % state->menu_count;
    } else if (action == INPUT_ENTER) {
        if (create_link(zapret_path, &state->menu_index) != 0) {
            set_error_window(
                state,
                "ERROR\nFailed to apply selected profile.\nSee terminal output for details."
            );
            return;
        }

        int compare_result = selected_config_matches_zapret(state->menu_index);
        if (compare_result != 0) {
            if (write_conf_sec("set_int", "view_profile", "-1") != 0) {
                set_error_window(
                    state,
                    "ERROR\nProfile check failed and state\ncould not be saved.\nSee terminal output."
                );
                return;
            }

            if (compare_result < 0) {
                snprintf(
                    state->error_message,
                    sizeof(state->error_message),
                    "ERROR\nFailed to verify selected\nconfig_%d against zapret2 config.\nSee terminal output.",
                    state->menu_index
                );
                state->error_window = true;
                return;
            }

            snprintf(
                state->error_message,
                sizeof(state->error_message),
                "ERROR\nzapret2 config does not match\nselected config_%d.\nProfile was not applied.",
                state->menu_index
            );
            state->error_window = true;
            return;
        }

        char view_profile_char[16];
        snprintf(view_profile_char, sizeof(view_profile_char), "%d", state->menu_index);
        if (write_conf_sec("set_int", "view_profile", view_profile_char) != 0) {
            set_error_window(
                state,
                "ERROR\nProfile was applied, but failed\nto save active profile.\nSee terminal output."
            );
            return;
        }

        state->view_profile = state->menu_index;
    } else if (action == INPUT_RELOAD_SERVICE) {
        if (zapret2_ctl(2) == 0) {
            state->is_reload = true;
        } else {
            state->service_error = true;
        }
    } else if (action == INPUT_START_SERVICE) {
        if (zapret2_ctl(0) != 0) state->service_error = true;
    } else if (action == INPUT_STOP_SERVICE) {
        if (zapret2_ctl(1) != 0) state->service_error = true;
    } else if (action == INPUT_OPEN_HELP_WINDOW) {
        state->help_window = !state->help_window;
    }
}

int zapret2_ctl(int code) {
    if (without_sudo) return 1;

    const char *cmd = NULL;
    switch (code) {
        case 0: cmd = "sudo systemctl start zapret2.service"; break;
        case 1: cmd = "sudo systemctl stop zapret2.service"; break;
        case 2: cmd = "sudo systemctl restart zapret2.service"; break;
        default: return -1;
    }

    int status = system(cmd);
    if (status == -1) return -1;
    return WEXITSTATUS(status);
}

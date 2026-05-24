#include "../../include/ui/core.h"
#include <stdio.h>

static char *zapret_path;
static bool without_sudo;

void core_init(AppState *state, short *n_profiles, char *get_zapret_path, bool get_without_sudo) {
    state->menu_index = 0;
    state->menu_count = *n_profiles;
    state->running = 1;

    zapret_path = get_zapret_path;
    without_sudo = get_without_sudo;
}

void core_update(AppState *state, InputAction action) {
    if (action == INPUT_QUIT) {
        state->running = 0;
        return;
    }

    if (action == INPUT_UP) {
        state->menu_index = (state->menu_index - 1 + state->menu_count) % state->menu_count;
    } else if (action == INPUT_DOWN) {
        state->menu_index = (state->menu_index + 1) % state->menu_count;
    } else if (action == INPUT_ENTER) {
        create_link(zapret_path, &state->menu_index);

        char view_profile_char[16];
        snprintf(view_profile_char, sizeof(view_profile_char), "%d", state->menu_index);
        write_conf_sec("set_int", "view_profile", view_profile_char);

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

#include "../../include/ui/core.h"

void core_init(AppState *state, short *n_profiles) {
    state->menu_index = 0;
    state->menu_count = *n_profiles;
    state->running = 1;
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
    } else if (action == INPUT_LEFT) {
        // state->scrol_opts_index = (state->scrol_opts_index - 1 + state->scrol_opts_count) % state->menu_count;
    } else if (action == INPUT_RIGHT) {
        // state->scrol_opts_index = (state->scrol_opts_index + 1) % state->scrol_opts_count;
    } else if (action == INPUT_ENTER) {
        
    } 
}

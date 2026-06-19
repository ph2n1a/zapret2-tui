#include "../../include/ui/core.h"
#include "../../include/utils/utils.h"
#include "../../include/utils/log.h"
#include <stdio.h>

static char *zapret_path;
static bool without_sudo;

static int testing_results[APP_MAX_PROFILES];
static TestingProfileJob testing_jobs[APP_MAX_PROFILES];
static pthread_mutex_t testing_mutex = PTHREAD_MUTEX_INITIALIZER;
static bool testing_all_running;

typedef struct {
    int profile_count;
    int *results;
    TestingProfileJob *jobs;
    pthread_mutex_t *mutex;
} TestingAllProfilesJob;

static TestingAllProfilesJob testing_all_job;

static void set_error_window(AppState *state, const char *message) {
    LOG_WARN("core", "Error: %s", message);
    state->error_window = true;
    snprintf(state->error_message, sizeof(state->error_message), "%s", message);
}

static void refresh_testing_profiles(AppState *state) {
    pthread_mutex_lock(&testing_mutex);
    for (int i = 0; i < state->menu_count && i < APP_MAX_PROFILES; i++) {
        if (state->testing_profiles[i] != testing_results[i]) {
            state->testing_profiles[i] = testing_results[i];
        }
    }
    pthread_mutex_unlock(&testing_mutex);
}

static bool zapret2_is_active(void) {
    int status = system("systemctl is-active --quiet zapret2.service >/dev/null 2>&1");
    return status != -1 && WIFEXITED(status) && WEXITSTATUS(status) == 0;
}

static bool testing_is_running_locked(void) {
    bool is_running = testing_all_running;

    for (int i = 0; i < APP_MAX_PROFILES && !is_running; i++) {
        is_running = testing_results[i] == TESTING_PROFILE_RUNNING;
    }

    return is_running;
}

static void set_testing_requires_stopped_error(AppState *state) {
    set_error_window(
        state,
        "ERROR\nStop zapret2 before testing.\nTesting requires zapret2 to be stopped."
    );
}

static void* testing_all_profiles(void* arg) {
    LOG_INFO("core", "Starting all profiles test");
    TestingAllProfilesJob *job = arg;
    if (!job || !job->results || !job->jobs || !job->mutex) {
        pthread_mutex_lock(&testing_mutex);
        testing_all_running = false;
        pthread_mutex_unlock(&testing_mutex);
        return NULL;
    }

    for (int i = 0; i < job->profile_count; i++) {
        pthread_mutex_lock(job->mutex);
        job->results[i] = TESTING_PROFILE_RUNNING;
        job->jobs[i].profile_index = i;
        job->jobs[i].results = job->results;
        job->jobs[i].mutex = job->mutex;
        pthread_mutex_unlock(job->mutex);

        if (i > 0) {
            usleep(500000);
        }

        testing_profiles(&job->jobs[i]);
    }

    pthread_mutex_lock(job->mutex);
    testing_all_running = false;
    pthread_mutex_unlock(job->mutex);

    return NULL;
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

void core_init(AppState *state, short *n_profiles, Profile *profiles, char *get_zapret_path, bool get_without_sudo, Testing testing_get) {
    LOG_INFO("core", "Core initialized, profiles=%d", *n_profiles);
    state->menu_index = 0;
    state->menu_count = (*n_profiles > APP_MAX_PROFILES) ? APP_MAX_PROFILES : *n_profiles;
    state->running = 1;
    state->error_window = false;
    state->error_message[0] = '\0';

    testing_log_open();

    pthread_mutex_lock(&testing_mutex);
    for (int i = 0; i < APP_MAX_PROFILES; i++) {
        testing_results[i] = TESTING_PROFILE_NOT_TESTED;
        state->testing_profiles[i] = TESTING_PROFILE_NOT_TESTED;
        testing_jobs[i].profile_index = i;
        if (profiles && i < state->menu_count) {
            testing_jobs[i].profiles = profiles[i];
        }
        testing_jobs[i].zapret_path = get_zapret_path;
        testing_jobs[i].results = testing_results;
        testing_jobs[i].mutex = &testing_mutex;
        testing_jobs[i].testing = testing_get;
    }
    testing_all_running = false;
    pthread_mutex_unlock(&testing_mutex);

    testing_all_job.profile_count = state->menu_count;
    testing_all_job.results = testing_results;
    testing_all_job.jobs = testing_jobs;
    testing_all_job.mutex = &testing_mutex;

    zapret_path = get_zapret_path;
    without_sudo = get_without_sudo;
}

void core_update(AppState *state, InputAction action) {
    if (action == INPUT_QUIT) {
        state->running = 0;
        return;
    }

    refresh_testing_profiles(state);

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
    } else if (action == INPUT_TESTING_ONE) {
            LOG_INFO("core", "Testing single profile %d", state->menu_index);
        int profile_index = state->menu_index;
        if (profile_index < 0 || profile_index >= APP_MAX_PROFILES) return;
        if (zapret2_is_active()) {
            set_testing_requires_stopped_error(state);
            return;
        }

        pthread_mutex_lock(&testing_mutex);
        bool can_start = !testing_is_running_locked();
        pthread_mutex_unlock(&testing_mutex);

        if (can_start) {
            state->testing_profiles[profile_index] = TESTING_PROFILE_RUNNING;

            pthread_mutex_lock(&testing_mutex);
            testing_results[profile_index] = TESTING_PROFILE_RUNNING;
            testing_jobs[profile_index].profile_index = profile_index;
            testing_jobs[profile_index].results = testing_results;
            testing_jobs[profile_index].mutex = &testing_mutex;
            pthread_mutex_unlock(&testing_mutex);

            pthread_t thread;
            if (pthread_create(&thread, NULL, testing_profiles, &testing_jobs[profile_index]) != 0) {
                pthread_mutex_lock(&testing_mutex);
                testing_results[profile_index] = TESTING_PROFILE_ERROR;
                pthread_mutex_unlock(&testing_mutex);
                state->testing_profiles[profile_index] = TESTING_PROFILE_ERROR;
                set_error_window(
                    state,
                    "ERROR\nFailed to start profile test.\nSee terminal output for details."
                );
                return;
            }
            pthread_detach(thread);
        }
    } else if (action == INPUT_TESTING_ALL) {
        LOG_INFO("core", "Testing all profiles");
        if (zapret2_is_active()) {
            set_testing_requires_stopped_error(state);
            return;
        }

        pthread_mutex_lock(&testing_mutex);
        bool can_start = !testing_is_running_locked();
        if (can_start) {
            testing_all_running = true;
            testing_all_job.profile_count = state->menu_count;
            for (int i = 0; i < state->menu_count && i < APP_MAX_PROFILES; i++) {
                testing_results[i] = TESTING_PROFILE_RUNNING;
                state->testing_profiles[i] = TESTING_PROFILE_RUNNING;
            }
        }
        pthread_mutex_unlock(&testing_mutex);

        if (!can_start) return;

        pthread_t thread;
        if (pthread_create(&thread, NULL, testing_all_profiles, &testing_all_job) != 0) {
            pthread_mutex_lock(&testing_mutex);
            testing_all_running = false;
            for (int i = 0; i < state->menu_count && i < APP_MAX_PROFILES; i++) {
                testing_results[i] = TESTING_PROFILE_ERROR;
                state->testing_profiles[i] = TESTING_PROFILE_ERROR;
            }
            pthread_mutex_unlock(&testing_mutex);
            set_error_window(
                state,
                "ERROR\nFailed to start profile tests.\nSee terminal output for details."
            );
            return;
        }

        pthread_detach(thread);
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

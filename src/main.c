#include "../include/utils/readconf.h"
#include "../include/utils/check_dependencies.h"
#include "../include/core/first_start.h"
#include "../include/core/read_zapret_conf.h"
#include "../include/core/create_zapret_configs.h"
#include "../include/utils/utils.h"
#include "../include/ui/app.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int main() {
  short n_profiles;
  short error_code = 0;

  if (!check_dependencies()) {
    return 1;
  }

  Config config_main;
  if (read_conf_config(&config_main, "main") != 0) {
    return 1;
  }

  if (config_main.first_start == true) {
    int first_start_result = first_start();
    if (first_start_result > 0) return 0;
    if (first_start_result < 0) return 1;
  }

  if (read_conf_config(&config_main, "main") != 0) {
    return 1;
  }
  if (strcmp(config_main.zapret_path, "none") == 0 || strcmp(config_main.program_path, "none") == 0) {
    fprintf(stderr, "Error: zapret path or program path is not configured in ./config/config.\n");
    return 1;
  }

  ZapretConf zapret_conf = read_conf_engine(config_main.zapret_path);
  Profile *profile = read_conf_profiles("profile", &n_profiles, &error_code);

  if (error_code || !profile) {
    return 1;
  }

  if (create_zapret_configs(zapret_conf, profile, &n_profiles)) {
    free(profile);
    return 1;
  }

  if (config_main.view_profile != -1) {
    char n_config_path[512];
    char zapret_config_path[512];
    int saved_view_profile = config_main.view_profile;

	    if (snprintf(n_config_path, sizeof(n_config_path), "%s/config/zapret_config/config_%d", config_main.program_path, saved_view_profile) >= (int)sizeof(n_config_path) ||
	        snprintf(zapret_config_path, sizeof(zapret_config_path), "%s/config", config_main.zapret_path) >= (int)sizeof(zapret_config_path)) {
	      config_main.view_profile = -1;
	      if (write_conf_sec("set_int", "view_profile", "-1") != 0) {
          free(profile);
          return 1;
        }
	      fprintf(stderr, "Warning: active profile was reset because its config path is too long.\n");
	    } else {
	      int compare_files_code = compare_files(zapret_config_path, n_config_path);

	      if (compare_files_code) {
	        config_main.view_profile = -1;
	        if (write_conf_sec("set_int", "view_profile", "-1") != 0) {
            free(profile);
            return 1;
          }
	        fprintf(stderr, "Warning: active profile was reset because zapret2 config does not match stored config_%d.\n",
                  saved_view_profile);
	      }
	    }
	  } 

  config_main.without_sudo = !is_root();

  if (app_init(profile, &n_profiles, config_main) != 0) {
    fprintf(stderr, "Error: failed to initialize TUI.\n");
    free(profile);
    return 1;
  }
  app_run();
  app_cleanup();
  free(profile);

  return 0;
}

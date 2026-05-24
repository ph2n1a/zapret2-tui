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
    printf("check_dependencies returned 1");
    return 1;
  }

  Config config_main;
  read_conf_config(&config_main, "main");

  if (config_main.first_start == true) {
    if (first_start()) return 0;
  }

  read_conf_config(&config_main, "main");
  if (strcmp(config_main.zapret_path, "none") == 0 || strcmp(config_main.program_path, "none") == 0) {
    printf("Error. You did not specify the path to the zapret2 folder.");
    return 1;
  }

  ZapretConf zapret_conf = read_conf_engine(config_main.zapret_path);
  Profile *profile = read_conf_profiles("profile", &n_profiles, &error_code);

  if (error_code) {
    printf("Error. Unable to read profies.conf. Possible errors:\n- Empty file\n- Syntax error\n- Invalid data type\n- Unknown parameter\n- Missing required parameter\n");
    return 1;
  }

  if (create_zapret_configs(zapret_conf, profile, &n_profiles)) {
    printf("Error. When program was creating zapret configs for links");
    return 1;
  }

  if (config_main.view_profile != -1) {
    char n_config_path[512];
    char zapret_config_path[512];

    snprintf(n_config_path, sizeof(n_config_path), "%s/config/zapret_config/config_%d", config_main.program_path, config_main.view_profile);
    snprintf(zapret_config_path, sizeof(zapret_config_path), "%s/config", config_main.zapret_path);
    int compare_files_code = compare_files(zapret_config_path, n_config_path);

    if (compare_files_code) {
      config_main.view_profile = -1;
      write_conf_sec("set_int", "view_profile", "-1");
      printf("View profil was set -1");
    }
  } 

  config_main.without_sudo = !is_root();

  if (app_init(profile, &n_profiles, config_main) != 0) {
    fprintf(stderr, "Failed to initialize TUI\n");
    return 1;
  }
  app_run();
  app_cleanup();

  return 0;
}

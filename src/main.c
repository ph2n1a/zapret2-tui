#include "../include/utils/readconf.h"
#include "../include/utils/check_dependencies.h"
#include "../include/core/first_start.h"
#include "../include/core/read_zapret_conf.h"
#include "../include/utils/utils.h"
#include "../include/core/create_zapret_configs.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

int main() {
  if (!check_dependencies()) {
    printf("check_dependencies returned 1");
    return 1;
  }

  Config config_main;
  read_conf_config(&config_main, "main");

  if (config_main.first_start == true) {
    if (first_start()) return 0;
  }

  if (strcmp(config_main.zapret_path, "none") == 0 || strcmp(config_main.program_path, "none") == 0) {
    printf("Error. You did not specify the path to the zapret2 folder.");
    return 1;
  }

  ZapretConf zapret_conf = read_conf_engine();
  
  short n_profiles;
  short error_code = 0;
  Profile *profiles = read_conf_profiles("profile", &n_profiles, &error_code);
  Profile profile[n_profiles];

  if (error_code) {
    printf("Error. Unable to read profies.conf. Possible errors:\n- Empty file\n- Syntax error\n- Invalid data type\n- Unknown parameter\n- Missing required parameter\n");
    return 1;
  }

  for (int i = 0; i < n_profiles; i++) {
    profile[i] = profiles[i];
    collapse_spaces(profile[i].nfqws2_opt);
    remove_newlines(profile[i].nfqws2_opt);
    printf("id: %d\nname: %s\nNFQWS2_OPT: %s\n\n", profile[i].id, profile[i].name, profile[i].nfqws2_opt);
  }

  free(profiles);
  
  if (create_zapret_configs(zapret_conf, profile, &n_profiles)) {
    printf("Error. When program was creating zapret configs for links");
    return 1;
  }

  printf("\nProgram finished!");
  return 0;
}

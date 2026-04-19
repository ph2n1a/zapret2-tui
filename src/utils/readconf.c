#include "../../include/utils/readconf.h"

int read_conf_sec(Config *config_main) {
  cfg_opt_t main_opts[] = {
    CFG_BOOL("first_start", cfg_true, CFGF_NONE),
    CFG_STR("zapret_path", "/opt/zapret2", CFGF_NONE),
    CFG_END()
  };

  cfg_opt_t opts[] = {
    CFG_SEC("main", main_opts, CFGF_NONE),
    CFG_END()
  };

  cfg_t *cfg = cfg_init(opts, 0);

  // читаем файл
  if (cfg_parse(cfg, "./config/config") == CFG_PARSE_ERROR) {
      printf("Parse error\n");
      return 1;
  }

  // получаем секцию
  cfg_t *main_sec = cfg_getsec(cfg, "main");

  // читаем значения
  config_main->first_start = cfg_getbool(main_sec, "first_start");
  strcpy(config_main->zapret_path, cfg_getstr(main_sec, "zapret_path"));

  return 0;
}


/* #include "../../include/utils/readconf.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

int handler(void *user, const char *section, const char *name, const char *value) {
  Config *config = (Config *)user;

  if (strcmp(section, "main") == 0) {
    if (strcmp(name, "first_start") == 0) {
      if (strcmp(value, "true") == 0) {
        config->first_start = true;
      } else if (strcmp(value, "false") == 0) {
        config->first_start = false;
      } else {
        printf("Error. Unable to read 'first_start' in src/config.");
      }
    } else if (strcmp(name, "zapret_path") == 0) {
      if (strcmp(value, "none") == 0) {
        strcpy(config->zapret_path, value);
        // printf("Error. You did not specify the path in 'zapret_path'.");
      } else {
        strcpy(config->zapret_path, value);
      }
    } else {
      printf("Error. Not found configs in src/config.");
    }
  } else {
    printf("Error. Unable to read src/config.");
  } 

  return 1;
} */

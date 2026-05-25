#include "../../include/utils/writeconf.h"

int write_conf_sec(const char *cfg_set, const char *name, const char *value) {
  cfg_opt_t main_opts[] = {
    CFG_BOOL("first_start", cfg_true, CFGF_NONE),
    CFG_STR("zapret_path", NULL, CFGF_NONE),
    CFG_STR("program_path", NULL, CFGF_NONE),
    CFG_INT("view_profile", -1, CFGF_NONE),
    CFG_END()
  };

  cfg_opt_t opts[] = {
    CFG_SEC("main", main_opts, CFGF_NONE),
    CFG_END()
  };

  cfg_t *cfg = cfg_init(opts, 0);

  if (cfg_parse(cfg, "./config/config") != CFG_SUCCESS) {
    printf("Parse error\n");
    cfg_free(cfg);
    return 1;
  }

  cfg_t *main_sec = cfg_getsec(cfg, "main");
  if (!main_sec) {
    printf("Parse error\n");
    cfg_free(cfg);
    return 1;
  }

  if (strcmp(cfg_set, "set_bool") == 0) {
    if (strcmp(value, "true") == 0) {
      cfg_setbool(main_sec, name, cfg_true);
    } else if (strcmp(value, "false") == 0) {
      cfg_setbool(main_sec, name, cfg_false);
    }
  } else if (strcmp(cfg_set, "set_str") == 0) {
    cfg_setstr(main_sec, name, value);
  } else if (strcmp(cfg_set, "set_int") == 0) {
    int num = strtol(value, NULL, 10);
    cfg_setint(main_sec, name, num);
  }

  FILE *f = fopen("./config/config", "w");
  if (!f) {
    printf("Error. Unable to write ./config/config\n");
    cfg_free(cfg);
    return 1;
  }

  cfg_print(cfg, f);
  fclose(f);

  cfg_free(cfg);

  return 0;
}

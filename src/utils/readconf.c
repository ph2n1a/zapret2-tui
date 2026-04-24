#include "../../include/utils/readconf.h"

int read_conf_sec(Config *config_main) {
  cfg_opt_t main_opts[] = {
    CFG_BOOL("first_start", cfg_true, CFGF_NONE),
    CFG_STR("zapret_path", "none", CFGF_NONE),
    CFG_STR("program_path", "none", CFGF_NONE),
    CFG_END()
  };

  cfg_opt_t opts[] = {
    CFG_SEC("main", main_opts, CFGF_NONE),
    CFG_END()
  };

  cfg_t *cfg = cfg_init(opts, 0);

  if (cfg_parse(cfg, "./config/config") == CFG_PARSE_ERROR) {
      printf("Parse error\n");
      return 1;
  }

  cfg_t *main_sec = cfg_getsec(cfg, "main");

  config_main->first_start = cfg_getbool(main_sec, "first_start");
  strcpy(config_main->zapret_path, cfg_getstr(main_sec, "zapret_path"));

  return 0;
}

char* read_conf_char(char *name) {
  cfg_opt_t main_opts[] = {
    CFG_BOOL("first_start", cfg_true, CFGF_NONE),
    CFG_STR("zapret_path", "none", CFGF_NONE),
    CFG_STR("program_path", "none", CFGF_NONE),
    CFG_END()
  };

  cfg_opt_t opts[] = {
    CFG_SEC("main", main_opts, CFGF_NONE),
    CFG_END()
  };

  cfg_t *cfg = cfg_init(opts, 0);

  if (cfg_parse(cfg, "./config/config") == CFG_PARSE_ERROR) {
      printf("Parse error\n");
  }

  cfg_t *main_sec = cfg_getsec(cfg, "main");

  return cfg_getstr(main_sec, name);
}

bool read_conf_bool(char *name) {
  cfg_opt_t main_opts[] = {
    CFG_BOOL("first_start", cfg_true, CFGF_NONE),
    CFG_STR("zapret_path", "none", CFGF_NONE),
    CFG_STR("program_path", "none", CFGF_NONE),
    CFG_END()
  };

  cfg_opt_t opts[] = {
    CFG_SEC("main", main_opts, CFGF_NONE),
    CFG_END()
  };

  cfg_t *cfg = cfg_init(opts, 0);

  if (cfg_parse(cfg, "./config/config") == CFG_PARSE_ERROR) {
      printf("Parse error\n");
  }

  cfg_t *main_sec = cfg_getsec(cfg, "main");

  return cfg_getbool(main_sec, name);
}

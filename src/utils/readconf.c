#include "../../include/utils/readconf.h"

int read_conf_config(Config *config_main, const char *section) {
  cfg_opt_t main_opts[] = {
    CFG_BOOL("first_start", cfg_true, CFGF_NONE),
    CFG_STR("zapret_path", "none", CFGF_NONE),
    CFG_STR("program_path", "none", CFGF_NONE),
    CFG_END()
  };

  if (strcmp(section, "main") == 0) {
    cfg_opt_t opts[] = {
      CFG_SEC(section, main_opts, CFGF_NONE),
      CFG_END()
    };

    cfg_t *cfg = cfg_init(opts, 0);

    if (cfg_parse(cfg, "./config/config") == CFG_PARSE_ERROR) {
      printf("Parse error\n");
    }

    cfg_t *main_sec = cfg_getsec(cfg, "main");

    config_main->first_start = cfg_getbool(main_sec, "first_start");
    strcpy(config_main->zapret_path, cfg_getstr(main_sec, "zapret_path"));
    strcpy(config_main->program_path, cfg_getstr(main_sec, "program_path"));

    cfg_free(cfg);
    return 0;
  } else {
    printf("Error. Section in ./config/config not founded");
    return 1;
  }
}

Profile* read_conf_profiles(const char *section, short *count, short *error_code) {
  Profile *profile = malloc(128 * sizeof(Profile));

  cfg_opt_t profile_opts[] = {
    CFG_INT("id", 0, CFGF_NONE),
    CFG_STR("name", "none", CFGF_NONE),
    CFG_STR("NFQWS2_OPT", "none", CFGF_NONE),
    CFG_END()
  };

  if (strcmp(section, "profile") == 0) {
    cfg_opt_t opts[] = {
      CFG_SEC(section, profile_opts, CFGF_MULTI),
      CFG_END()
    };

    cfg_t *cfg = cfg_init(opts, 0);

    int ret = cfg_parse(cfg, "./profiles.conf");
    if (ret == CFG_PARSE_ERROR || ret != CFG_SUCCESS) {
      *error_code = 1;
      printf("Parse error\n");
      return NULL;
    }

    short n_profiles = cfg_size(cfg, section);
    *count = n_profiles;

    if (n_profiles == 0) {
      *error_code = 1;
      return NULL;
    }

    for (short i = 0; i < n_profiles; i++) {
      cfg_t *sec = cfg_getnsec(cfg, section, i);
      profile[i].id = cfg_getint(sec, "id");

      if (i > 0 && profile[i].id == 0) {
        *error_code = 1;
        printf("Parse error\n");
        return NULL;
      }

      strcpy(profile[i].name, cfg_getstr(sec, "name"));
      strcpy(profile[i].nfqws2_opt, cfg_getstr(sec, "NFQWS2_OPT"));
      
      if (strcmp(profile[i].name, "none") == 0 || strcmp(profile[i].nfqws2_opt, "none") == 0) {
        *error_code = 1;
        return NULL;
      }
    }

    cfg_free(cfg);
  } else {
    printf("Error. Section in ./profiles.conf not founded");
  }

  return profile;
}



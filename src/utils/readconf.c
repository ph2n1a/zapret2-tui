#include "../../include/utils/readconf.h"

int read_conf_config(Config *config_main, const char *section) {
  cfg_opt_t main_opts[] = {
    CFG_BOOL("first_start", cfg_true, CFGF_NONE),
    CFG_STR("zapret_path", "none", CFGF_NONE),
    CFG_STR("program_path", "none", CFGF_NONE),
    CFG_INT("view_profile", -1, CFGF_NONE),
    CFG_END()
  };

  if (strcmp(section, "main") == 0) {
    cfg_opt_t opts[] = {
      CFG_SEC(section, main_opts, CFGF_NONE),
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

    config_main->first_start = cfg_getbool(main_sec, "first_start");
    const char *zapret_path = cfg_getstr(main_sec, "zapret_path");
    const char *program_path = cfg_getstr(main_sec, "program_path");
    if (!zapret_path || !program_path) {
      printf("Parse error\n");
      cfg_free(cfg);
      return 1;
    }

    if (strlen(zapret_path) >= sizeof(config_main->zapret_path) ||
        strlen(program_path) >= sizeof(config_main->program_path)) {
      printf("Error. Path too long in config\n");
      cfg_free(cfg);
      return 1;
    }

    snprintf(config_main->zapret_path, sizeof(config_main->zapret_path), "%s", zapret_path);
    snprintf(config_main->program_path, sizeof(config_main->program_path), "%s", program_path);
    config_main->view_profile = cfg_getint(main_sec, "view_profile");

    cfg_free(cfg);
    return 0;
  } else {
    printf("Error. Section in ./config/config not founded");
    return 1;
  }
}

Profile* read_conf_profiles(const char *section, short *count, short *error_code) {
  if (!count || !error_code) return NULL;
  *count = 0;
  *error_code = 0;

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
    if (ret != CFG_SUCCESS) {
      *error_code = 1;
      printf("Parse error\n");
      cfg_free(cfg);
      return NULL;
    }

    short n_profiles = cfg_size(cfg, section);
    *count = n_profiles;

    if (n_profiles == 0) {
      *error_code = 1;
      cfg_free(cfg);
      return NULL;
    }

    if (n_profiles < 0 || n_profiles > 1024) {
      *error_code = 1;
      printf("Error. Too many profiles\n");
      cfg_free(cfg);
      return NULL;
    }

    Profile *profile = calloc((size_t)n_profiles, sizeof(Profile));
    if (!profile) {
      *error_code = 1;
      printf("Error. Out of memory\n");
      cfg_free(cfg);
      return NULL;
    }

    for (short i = 0; i < n_profiles; i++) {
      cfg_t *sec = cfg_getnsec(cfg, section, i);
      profile[i].id = cfg_getint(sec, "id");

      if (i > 0 && profile[i].id == 0) {
        *error_code = 1;
        printf("Parse error\n");
        free(profile);
        cfg_free(cfg);
        return NULL;
      }

      const char *name = cfg_getstr(sec, "name");
      const char *nfqws2_opt = cfg_getstr(sec, "NFQWS2_OPT");
      if (!name || !nfqws2_opt) {
        *error_code = 1;
        free(profile);
        cfg_free(cfg);
        return NULL;
      }

      if (strlen(name) >= sizeof(profile[i].name) ||
          strlen(nfqws2_opt) >= sizeof(profile[i].nfqws2_opt)) {
        *error_code = 1;
        printf("Error. Profile value too long\n");
        free(profile);
        cfg_free(cfg);
        return NULL;
      }

      snprintf(profile[i].name, sizeof(profile[i].name), "%s", name);
      snprintf(profile[i].nfqws2_opt, sizeof(profile[i].nfqws2_opt), "%s", nfqws2_opt);
      
      if (strcmp(profile[i].name, "none") == 0 || strcmp(profile[i].nfqws2_opt, "none") == 0) {
        *error_code = 1;
        free(profile);
        cfg_free(cfg);
        return NULL;
      }
    }

    cfg_free(cfg);
    return profile;
  } else {
    printf("Error. Section in ./profiles.conf not founded");
  }

  *error_code = 1;
  return NULL;
}


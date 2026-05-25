#include "../../include/utils/readconf.h"
#include <stdio.h>

int read_conf_config(Config *config_main, const char *section) {
  if (!config_main || !section) {
    fprintf(stderr, "Error: read_conf_config received invalid arguments.\n");
    return 1;
  }

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
      fprintf(stderr, "Error: failed to parse ./config/config.\n");
      cfg_free(cfg);
      return 1;
    }

    cfg_t *main_sec = cfg_getsec(cfg, "main");
    if (!main_sec) {
      fprintf(stderr, "Error: section \"main\" is missing in ./config/config.\n");
      cfg_free(cfg);
      return 1;
    }

    config_main->first_start = cfg_getbool(main_sec, "first_start");
    const char *zapret_path = cfg_getstr(main_sec, "zapret_path");
    const char *program_path = cfg_getstr(main_sec, "program_path");
    if (!zapret_path || !program_path) {
      fprintf(stderr, "Error: required keys are missing in ./config/config.\n");
      cfg_free(cfg);
      return 1;
    }

    if (strlen(zapret_path) >= sizeof(config_main->zapret_path) ||
        strlen(program_path) >= sizeof(config_main->program_path)) {
      fprintf(stderr, "Error: zapret_path or program_path is too long in ./config/config.\n");
      cfg_free(cfg);
      return 1;
    }

    snprintf(config_main->zapret_path, sizeof(config_main->zapret_path), "%s", zapret_path);
    snprintf(config_main->program_path, sizeof(config_main->program_path), "%s", program_path);
    config_main->view_profile = cfg_getint(main_sec, "view_profile");

    cfg_free(cfg);
    return 0;
  } else {
    fprintf(stderr, "Error: unsupported config section \"%s\" for ./config/config.\n", section);
    return 1;
  }
}

Profile* read_conf_profiles(const char *section, short *count, short *error_code) {
  if (!section || !count || !error_code) {
    fprintf(stderr, "Error: read_conf_profiles received invalid arguments.\n");
    return NULL;
  }
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
      fprintf(stderr, "Error: failed to parse ./profiles.conf.\n");
      cfg_free(cfg);
      return NULL;
    }

    short n_profiles = cfg_size(cfg, section);
    *count = n_profiles;

    if (n_profiles == 0) {
      *error_code = 1;
      fprintf(stderr, "Error: ./profiles.conf does not contain any profile sections.\n");
      cfg_free(cfg);
      return NULL;
    }

    if (n_profiles < 0 || n_profiles > 1024) {
      *error_code = 1;
      fprintf(stderr, "Error: too many profiles in ./profiles.conf (max 1024).\n");
      cfg_free(cfg);
      return NULL;
    }

    Profile *profile = calloc((size_t)n_profiles, sizeof(Profile));
    if (!profile) {
      *error_code = 1;
      fprintf(stderr, "Error: out of memory while reading ./profiles.conf.\n");
      cfg_free(cfg);
      return NULL;
    }

    for (short i = 0; i < n_profiles; i++) {
      cfg_t *sec = cfg_getnsec(cfg, section, i);
      if (!sec) {
        *error_code = 1;
        fprintf(stderr, "Error: failed to read profile section %d from ./profiles.conf.\n", i);
        free(profile);
        cfg_free(cfg);
        return NULL;
      }

      profile[i].id = cfg_getint(sec, "id");

      if (profile[i].id != i) {
        *error_code = 1;
        fprintf(stderr,
                "Error: invalid profile id at index %d: expected %d, got %d. "
                "Profile ids in ./profiles.conf must be sequential and start at 0.\n",
                i, i, profile[i].id);
        free(profile);
        cfg_free(cfg);
        return NULL;
      }

      const char *name = cfg_getstr(sec, "name");
      const char *nfqws2_opt = cfg_getstr(sec, "NFQWS2_OPT");
      if (!name || !nfqws2_opt) {
        *error_code = 1;
        fprintf(stderr, "Error: profile %d is missing required keys \"name\" or \"NFQWS2_OPT\".\n", i);
        free(profile);
        cfg_free(cfg);
        return NULL;
      }

      if (strlen(name) >= sizeof(profile[i].name) ||
          strlen(nfqws2_opt) >= sizeof(profile[i].nfqws2_opt)) {
        *error_code = 1;
        fprintf(stderr, "Error: profile %d contains a value that is too long.\n", i);
        free(profile);
        cfg_free(cfg);
        return NULL;
      }

      snprintf(profile[i].name, sizeof(profile[i].name), "%s", name);
      snprintf(profile[i].nfqws2_opt, sizeof(profile[i].nfqws2_opt), "%s", nfqws2_opt);
      
      if (strcmp(profile[i].name, "none") == 0 || strcmp(profile[i].nfqws2_opt, "none") == 0) {
        *error_code = 1;
        fprintf(stderr, "Error: profile %d contains placeholder value \"none\" in a required field.\n", i);
        free(profile);
        cfg_free(cfg);
        return NULL;
      }
    }

    cfg_free(cfg);
    return profile;
  } else {
    fprintf(stderr, "Error: unsupported section \"%s\" for ./profiles.conf.\n", section);
  }

  *error_code = 1;
  return NULL;
}

#include "../../include/utils/readconf.h"
#include "../../include/utils/log.h"
#include <confuse.h>
#include <stdio.h>

int read_conf_config(Config *config_main, const char *section) {
  LOG_INFO("readconf", "Reading config section '%s'", section);
  if (!config_main || !section) {
    LOG_ERROR("readconf", "read_conf_config: invalid arguments");
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
      LOG_ERROR("readconf", "Failed to parse ./config/config");
      cfg_free(cfg);
      return 1;
    }

    cfg_t *main_sec = cfg_getsec(cfg, "main");
    if (!main_sec) {
      LOG_ERROR("readconf", "Section 'main' missing in config");
      cfg_free(cfg);
      return 1;
    }

    config_main->first_start = cfg_getbool(main_sec, "first_start");
    const char *zapret_path = cfg_getstr(main_sec, "zapret_path");
    const char *program_path = cfg_getstr(main_sec, "program_path");
    if (!zapret_path || !program_path) {
      LOG_ERROR("readconf", "Required keys missing in config");
      cfg_free(cfg);
      return 1;
    }

    if (strlen(zapret_path) >= sizeof(config_main->zapret_path) ||
        strlen(program_path) >= sizeof(config_main->program_path)) {
      LOG_ERROR("readconf", "zapret_path or program_path too long");
      cfg_free(cfg);
      return 1;
    }

    snprintf(config_main->zapret_path, sizeof(config_main->zapret_path), "%s", zapret_path);
    snprintf(config_main->program_path, sizeof(config_main->program_path), "%s", program_path);
    config_main->view_profile = cfg_getint(main_sec, "view_profile");

    cfg_free(cfg);
    return 0;
  } else {
    LOG_ERROR("readconf", "Unsupported section '%s'", section);
    return 1;
  }
}

Profile* read_conf_profiles(const char *section, short *count, short *error_code) {
  LOG_INFO("readconf", "Reading profiles from ./profiles.conf");
  if (!section || !count || !error_code) {
    LOG_ERROR("readconf", "read_conf_profiles: invalid arguments");
    return NULL;
  }
  *count = 0;
  *error_code = 0;

  cfg_opt_t testing_opts[] = {
    CFG_STR("tables", "iptables", CFGF_NONE),
    CFG_STR("domain_test", "rutracker.org", CFGF_NONE),
    CFG_END()
  };

  cfg_opt_t profile_opts[] = {
    CFG_INT("id", 0, CFGF_NONE),
    CFG_STR("name", "none", CFGF_NONE),
    CFG_STR("NFQWS2_OPT", "none", CFGF_NONE),
    CFG_END()
  };

  if (strcmp(section, "profile") == 0) {
    cfg_opt_t opts[] = {
      CFG_SEC("profile", profile_opts, CFGF_MULTI),
      CFG_SEC("testing", testing_opts, CFGF_NONE),
      CFG_END()
    };

    cfg_t *cfg = cfg_init(opts, 0);

    int ret = cfg_parse(cfg, "./profiles.conf");
    if (ret != CFG_SUCCESS) {
      *error_code = 1;
      LOG_ERROR("readconf", "Failed to parse ./profiles.conf");
      cfg_free(cfg);
      return NULL;
    }

    short n_profiles = cfg_size(cfg, section);
    *count = n_profiles;

    if (n_profiles == 0) {
      *error_code = 1;
      LOG_ERROR("readconf", "No profile sections in ./profiles.conf");
      cfg_free(cfg);
      return NULL;
    }

    if (n_profiles < 0 || n_profiles > 1024) {
      *error_code = 1;
      LOG_ERROR("readconf", "Too many profiles (max 1024)");
      cfg_free(cfg);
      return NULL;
    }

    Profile *profile = calloc((size_t)n_profiles, sizeof(Profile));
    if (!profile) {
      *error_code = 1;
      LOG_ERROR("readconf", "Out of memory reading profiles");
      cfg_free(cfg);
      return NULL;
    }

    for (short i = 0; i < n_profiles; i++) {
      cfg_t *sec = cfg_getnsec(cfg, section, i);
      if (!sec) {
        *error_code = 1;
        LOG_ERROR("readconf", "Failed to read profile section %d", i);
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
        LOG_ERROR("readconf", "Profile %d value too long", i);
        free(profile);
        cfg_free(cfg);
        return NULL;
      }

      snprintf(profile[i].name, sizeof(profile[i].name), "%s", name);
      snprintf(profile[i].nfqws2_opt, sizeof(profile[i].nfqws2_opt), "%s", nfqws2_opt);
      
      if (strcmp(profile[i].name, "none") == 0 || strcmp(profile[i].nfqws2_opt, "none") == 0) {
        *error_code = 1;
        LOG_ERROR("readconf", "Profile %d has placeholder 'none' in required field", i);
        free(profile);
        cfg_free(cfg);
        return NULL;
      }
    }

    cfg_free(cfg);
    return profile;
  } else {
    LOG_ERROR("readconf", "Unsupported section '%s'", section);
  }

  *error_code = 1;
  return NULL;
}

Testing read_conf_testing() {
  LOG_INFO("readconf", "Reading testing config");
  Testing result;
  result.error_code = false;

  cfg_opt_t testing_opts[] = {
    CFG_STR("tables", "iptables", CFGF_NONE),
    CFG_STR("domain_test", "rutracker.org", CFGF_NONE),
    CFG_END()
  };

  cfg_opt_t profile_opts[] = {
    CFG_INT("id", 0, CFGF_NONE),
    CFG_STR("name", "none", CFGF_NONE),
    CFG_STR("NFQWS2_OPT", "none", CFGF_NONE),
    CFG_END()
  };

  cfg_opt_t opts[] = {
    CFG_SEC("profile", profile_opts, CFGF_MULTI),
    CFG_SEC("testing", testing_opts, CFGF_NONE),
    CFG_END()
  };

  cfg_t *cfg = cfg_init(opts, 0);

  if (cfg_parse(cfg, "./profiles.conf") != CFG_SUCCESS) {
    LOG_ERROR("readconf", "Failed to parse ./profiles.conf (testing)");
    cfg_free(cfg);
    result.error_code = true;
    return result;
  }

  cfg_t *testing_sec = cfg_getsec(cfg, "testing");
  if (!testing_sec) {
    LOG_ERROR("readconf", "Section 'testing' missing in ./profiles.conf");
    cfg_free(cfg);
    result.error_code = true;
    return result;
  }

  snprintf(result.tables, sizeof(result.tables), "%s", cfg_getstr(testing_sec, "tables"));
  snprintf(result.domain, sizeof(result.domain), "%s", cfg_getstr(testing_sec, "domain_test"));

  cfg_free(cfg);
  return result;
}

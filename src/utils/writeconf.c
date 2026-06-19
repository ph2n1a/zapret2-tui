#include "../../include/utils/writeconf.h"
#include "../../include/utils/log.h"
#include <errno.h>

int write_conf_sec(const char *cfg_set, const char *name, const char *value) {
  LOG_INFO("writeconf", "Writing %s=%s (mode: %s)", name, value, cfg_set);
  if (!cfg_set || !name || !value) {
    LOG_ERROR("writeconf", "Invalid arguments");
    return 1;
  }

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
    LOG_ERROR("writeconf", "Failed to parse config before writing");
    cfg_free(cfg);
    return 1;
  }

  cfg_t *main_sec = cfg_getsec(cfg, "main");
  if (!main_sec) {
    LOG_ERROR("writeconf", "Section 'main' missing");
    cfg_free(cfg);
    return 1;
  }

  if (strcmp(cfg_set, "set_bool") == 0) {
    if (strcmp(value, "true") == 0) {
      cfg_setbool(main_sec, name, cfg_true);
    } else if (strcmp(value, "false") == 0) {
      cfg_setbool(main_sec, name, cfg_false);
    } else {
      LOG_ERROR("writeconf", "Invalid boolean '%s' for key '%s'", value, name);
      cfg_free(cfg);
      return 1;
    }
  } else if (strcmp(cfg_set, "set_str") == 0) {
    cfg_setstr(main_sec, name, value);
  } else if (strcmp(cfg_set, "set_int") == 0) {
    int num = strtol(value, NULL, 10);
    cfg_setint(main_sec, name, num);
  } else {
    LOG_ERROR("writeconf", "Unsupported write mode '%s'", cfg_set);
    cfg_free(cfg);
    return 1;
  }

  FILE *f = fopen("./config/config", "w");
  if (!f) {
    LOG_ERROR("writeconf", "Failed to open config for writing: %s", strerror(errno));
    cfg_free(cfg);
    return 1;
  }

  cfg_print(cfg, f);
  if (fclose(f) != 0) {
    LOG_ERROR("writeconf", "Failed to flush config: %s", strerror(errno));
    cfg_free(cfg);
    return 1;
  }

  cfg_free(cfg);

  return 0;
}

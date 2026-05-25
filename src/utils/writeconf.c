#include "../../include/utils/writeconf.h"
#include <errno.h>

int write_conf_sec(const char *cfg_set, const char *name, const char *value) {
  if (!cfg_set || !name || !value) {
    fprintf(stderr, "Error: write_conf_sec received invalid arguments.\n");
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
    fprintf(stderr, "Error: failed to parse ./config/config before writing.\n");
    cfg_free(cfg);
    return 1;
  }

  cfg_t *main_sec = cfg_getsec(cfg, "main");
  if (!main_sec) {
    fprintf(stderr, "Error: section \"main\" is missing in ./config/config.\n");
    cfg_free(cfg);
    return 1;
  }

  if (strcmp(cfg_set, "set_bool") == 0) {
    if (strcmp(value, "true") == 0) {
      cfg_setbool(main_sec, name, cfg_true);
    } else if (strcmp(value, "false") == 0) {
      cfg_setbool(main_sec, name, cfg_false);
    } else {
      fprintf(stderr, "Error: invalid boolean value \"%s\" for key \"%s\".\n", value, name);
      cfg_free(cfg);
      return 1;
    }
  } else if (strcmp(cfg_set, "set_str") == 0) {
    cfg_setstr(main_sec, name, value);
  } else if (strcmp(cfg_set, "set_int") == 0) {
    int num = strtol(value, NULL, 10);
    cfg_setint(main_sec, name, num);
  } else {
    fprintf(stderr, "Error: unsupported write mode \"%s\".\n", cfg_set);
    cfg_free(cfg);
    return 1;
  }

  FILE *f = fopen("./config/config", "w");
  if (!f) {
    fprintf(stderr, "Error: failed to open ./config/config for writing: %s\n", strerror(errno));
    cfg_free(cfg);
    return 1;
  }

  cfg_print(cfg, f);
  if (fclose(f) != 0) {
    fprintf(stderr, "Error: failed to flush ./config/config: %s\n", strerror(errno));
    cfg_free(cfg);
    return 1;
  }

  cfg_free(cfg);

  return 0;
}

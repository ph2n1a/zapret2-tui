#include "../../include/core/create_zapret_configs.h"

int create_zapret_configs(ZapretConf zapret_config, Profile profile[], const short *n_profiles) {
  char path[128];

  if (!profile || !n_profiles) {
    fprintf(stderr, "Error: create_zapret_configs received invalid arguments.\n");
    return 1;
  }

  if (zapret_config.start < 0 || zapret_config.lines <= 0) {
    fprintf(stderr, "Error: zapret config template is invalid.\n");
    return 1;
  }

  for (short i = 0; i < *n_profiles; i++) {
    bool written = false;

    if (snprintf(path, sizeof(path), "./config/zapret_config/config_%d", profile[i].id) >= (int)sizeof(path)) {
      fprintf(stderr, "Error: generated config path is too long for profile %d.\n", profile[i].id);
      return 1;
    }
    FILE *f = fopen(path, "w");
    if (f == NULL) {
      fprintf(stderr, "Error: failed to open %s for writing.\n", path);
      return 1;
    }

    for (short j = 0; j < zapret_config.lines; j++) {
      if (zapret_config.stand_format) {
        if (j >= (zapret_config.start - 1) && j <= (zapret_config.finish + 1)) {
          if (!written) {
            if (fprintf(f, "NFQWS2_OPT=\"%s\"\n", profile[i].nfqws2_opt) < 0) {
              fprintf(stderr, "Error: failed to write generated config %s.\n", path);
              fclose(f);
              return 1;
            }
            written = true;
          }
        } else {
          if (fputs(zapret_config.text[j], f) == EOF) {
            fprintf(stderr, "Error: failed to write generated config %s.\n", path);
            fclose(f);
            return 1;
          }
        }
      } else {
        if (j == zapret_config.start) {
          if (!written) {
            if (fprintf(f, "NFQWS2_OPT=\"%s\"\n", profile[i].nfqws2_opt) < 0) {
              fprintf(stderr, "Error: failed to write generated config %s.\n", path);
              fclose(f);
              return 1;
            }
            written = true;
          }
        } else {
          if (fputs(zapret_config.text[j], f) == EOF) {
            fprintf(stderr, "Error: failed to write generated config %s.\n", path);
            fclose(f);
            return 1;
          }
        }
      }
    }
    if (fclose(f) != 0) {
      fprintf(stderr, "Error: failed to finalize generated config %s.\n", path);
      return 1;
    }
  }

  return 0;
}

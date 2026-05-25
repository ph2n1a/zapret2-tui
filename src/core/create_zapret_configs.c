#include "../../include/core/create_zapret_configs.h"

int create_zapret_configs(ZapretConf zapret_config, Profile profile[], const short *n_profiles) {
  char path[128];

  if (zapret_config.start < 0 || zapret_config.lines <= 0) {
    printf("Error. Invalid zapret config\n");
    return 1;
  }

  for (short i = 0; i < *n_profiles; i++) {
    bool written = false;

    snprintf(path, sizeof(path), "./config/zapret_config/config_%d", profile[i].id);
    FILE *f = fopen(path, "w");
    if (f == NULL) {
      printf("Error. With file %s", path);
      return 1;
    }

    for (short j = 0; j < zapret_config.lines; j++) {
      if (zapret_config.stand_format) {
        if (j >= (zapret_config.start - 1) && j <= (zapret_config.finish + 1)) {
          if (!written) {
            fprintf(f, "NFQWS2_OPT=\"%s\"\n", profile[i].nfqws2_opt);
            written = true;
          }
        } else {
          fputs(zapret_config.text[j], f);
        }
      } else {
        if (j == zapret_config.start) {
          if (!written) {
            fprintf(f, "NFQWS2_OPT=\"%s\"\n", profile[i].nfqws2_opt);
            written = true;
          }
        } else {
          fputs(zapret_config.text[j], f);
        }
      }
    }
    fclose(f);
  }

  return 0;
}

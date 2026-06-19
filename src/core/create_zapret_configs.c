#include "../../include/core/create_zapret_configs.h"
#include "../../include/utils/log.h"

int create_zapret_configs(ZapretConf zapret_config, Profile profile[], const short *n_profiles) {
  LOG_INFO("create_configs", "Generating zapret configs for %d profiles", *n_profiles);
  char path[128];

  if (!profile || !n_profiles) {
    LOG_ERROR("create_configs", "Received invalid arguments");
    return 1;
  }

  if (zapret_config.start < 0 || zapret_config.lines <= 0) {
    LOG_ERROR("create_configs", "Zapret config template is invalid");
    return 1;
  }

  for (short i = 0; i < *n_profiles; i++) {
    bool written = false;

    if (snprintf(path, sizeof(path), "./config/zapret_config/config_%d", profile[i].id) >= (int)sizeof(path)) {
      LOG_ERROR("create_configs", "Config path too long for profile %d", profile[i].id);
      return 1;
    }
    FILE *f = fopen(path, "w");
    if (f == NULL) {
      LOG_ERROR("create_configs", "Failed to open %s for writing", path);
      return 1;
    }

    for (short j = 0; j < zapret_config.lines; j++) {
      if (zapret_config.stand_format) {
        if (j >= (zapret_config.start - 1) && j <= (zapret_config.finish + 1)) {
          if (!written) {
            if (fprintf(f, "NFQWS2_OPT=\"%s\"\n", profile[i].nfqws2_opt) < 0) {
              LOG_ERROR("create_configs", "Failed to write config %s", path);
              fclose(f);
              return 1;
            }
            written = true;
          }
        } else {
          if (fputs(zapret_config.text[j], f) == EOF) {
            LOG_ERROR("create_configs", "Failed to write config %s", path);
            fclose(f);
            return 1;
          }
        }
      } else {
        if (j == zapret_config.start) {
          if (!written) {
            if (fprintf(f, "NFQWS2_OPT=\"%s\"\n", profile[i].nfqws2_opt) < 0) {
              LOG_ERROR("create_configs", "Failed to write config %s", path);
              fclose(f);
              return 1;
            }
            written = true;
          }
        } else {
          if (fputs(zapret_config.text[j], f) == EOF) {
            LOG_ERROR("create_configs", "Failed to write config %s", path);
            fclose(f);
            return 1;
          }
        }
      }
    }
    if (fclose(f) != 0) {
      LOG_ERROR("create_configs", "Failed to finalize config %s", path);
      return 1;
    }
  }

  return 0;
}

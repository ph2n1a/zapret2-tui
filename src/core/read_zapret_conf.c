#include "../../include/core/read_zapret_conf.h"
#include "../../include/utils/log.h"
#include <errno.h>
#include <limits.h>

ZapretConf read_conf_engine(char *get_zapret_path) {
  LOG_INFO("read_zapret_conf", "Reading zapret config from %s", get_zapret_path ? get_zapret_path : "NULL");
  ZapretConf info = {0};
  info.start = -1;
  info.finish = -1;
  info.lines = 0;
  info.stand_format = false;

  if (!get_zapret_path || !*get_zapret_path) {
    LOG_ERROR("read_zapret_conf", "Zapret path is empty");
    return info;
  }

  char zapret_conf_path[PATH_MAX];
  if (snprintf(zapret_conf_path, sizeof(zapret_conf_path), "%s/config", get_zapret_path) >= (int)sizeof(zapret_conf_path)) {
    LOG_ERROR("read_zapret_conf", "Config path too long");
    return info;
  }

  FILE *f = fopen(zapret_conf_path, "r");
  if (!f) {
    LOG_ERROR("read_zapret_conf", "Failed to open %s: %s", zapret_conf_path, strerror(errno));
    return info;
  }

  char line[512];
  short i = 0;
  while (fgets(line, sizeof(line), f) && i < 256) {
    snprintf(info.text[i], sizeof(info.text[i]), "%s", line);
    i++;
  }
  fclose(f);
  info.lines = i;

  for (int j = 0; j < info.lines; j++) {
    const char *s = info.text[j];
    if (!strstr(s, "NFQWS2_OPT=")) continue;

    const char *eq = strchr(s, '=');
    if (!eq) continue;

    const char *first_quote = strchr(eq + 1, '"');
    if (!first_quote) continue;

    const char *second_quote = strchr(first_quote + 1, '"');
    if (second_quote) {
      info.stand_format = false;
      info.start = j;
      info.finish = j;
      break;
    }

    info.stand_format = true;
    info.start = j + 1;

    for (int k = info.start; k < info.lines; k++) {
      if (strchr(info.text[k], '"')) {
        info.finish = k - 1;
        break;
      }
    }
    break;
  }

  if (info.start < 0) {
    LOG_ERROR("read_zapret_conf", "NFQWS2_OPT not found in %s", zapret_conf_path);
  } else if (info.stand_format && info.finish < info.start - 1) {
    LOG_ERROR("read_zapret_conf", "Failed to parse multi-line NFQWS2_OPT in %s", zapret_conf_path);
    info.start = -1;
    info.finish = -1;
    info.stand_format = false;
  }

  return info;
}

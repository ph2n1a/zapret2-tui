#include "../../include/core/read_zapret_conf.h"
#include <errno.h>
#include <limits.h>

ZapretConf read_conf_engine(char *get_zapret_path) {
  ZapretConf info = {0};
  info.start = -1;
  info.finish = -1;
  info.lines = 0;
  info.stand_format = false;

  if (!get_zapret_path || !*get_zapret_path) {
    fprintf(stderr, "Error: zapret path is empty.\n");
    return info;
  }

  char zapret_conf_path[PATH_MAX];
  if (snprintf(zapret_conf_path, sizeof(zapret_conf_path), "%s/config", get_zapret_path) >= (int)sizeof(zapret_conf_path)) {
    fprintf(stderr, "Error: zapret config path is too long.\n");
    return info;
  }

  FILE *f = fopen(zapret_conf_path, "r");
  if (!f) {
    fprintf(stderr, "Error: failed to open %s: %s\n", zapret_conf_path, strerror(errno));
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
    fprintf(stderr, "Error: NFQWS2_OPT was not found in %s.\n", zapret_conf_path);
  } else if (info.stand_format && info.finish < info.start - 1) {
    fprintf(stderr, "Error: failed to parse multi-line NFQWS2_OPT in %s.\n", zapret_conf_path);
    info.start = -1;
    info.finish = -1;
    info.stand_format = false;
  }

  return info;
}

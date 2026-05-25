#include "../../include/core/read_zapret_conf.h"

ZapretConf read_conf_engine(char *get_zapret_path) {
  ZapretConf info = {0};
  info.start = -1;
  info.finish = -1;
  info.lines = 0;
  info.stand_format = false;

  if (!get_zapret_path || !*get_zapret_path) {
    printf("Error. Empty zapret path\n");
    return info;
  }

  const char *suffix = "/config";
  size_t need = strlen(get_zapret_path) + strlen(suffix) + 1;
  char *zapret_conf_path = malloc(need);
  if (!zapret_conf_path) {
    printf("Error. Out of memory\n");
    return info;
  }
  snprintf(zapret_conf_path, need, "%s%s", get_zapret_path, suffix);

  FILE *f = fopen(zapret_conf_path, "r");
  free(zapret_conf_path);
  if (!f) {
    printf("Error opened file\n");
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
    printf("Error. NFQWS2_OPT not found\n");
  } else if (info.stand_format && info.finish < info.start - 1) {
    printf("Error. Failed to read NFQWS2_OPT correctly\n");
    info.start = -1;
    info.finish = -1;
    info.stand_format = false;
  }

  return info;
}

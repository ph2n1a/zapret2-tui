#include "../../include/core/read_zapret_conf.h"

ZapretConf read_conf_engine(char *get_zapret_path) {
  char *zapret_path = strdup(get_zapret_path);
  strcat(zapret_path, "/config");
  FILE *f = fopen(zapret_path, "r");

  ZapretConf info;
  char text[256][512];
  char line[512];
  char nfqws2_opt[64][512];
  int i = 0;
  int c = 0;
  int start = 0;
  int finish = 0;
  int stand_format = 2;
  bool start_scan = false;

  if (f != NULL) {
    while (fgets(line, sizeof(line), f) && i < 256) {
      strcpy(text[i], line);
        i++;
      }
      fclose(f);
  } else {
    printf("Error opened file\n");
    exit(1);
  }

  for (int j = 0; j < i; j++) {
    int len = strlen(text[j]);

    if (start_scan && stand_format == 1) {
      if (strstr(text[j], "\"") != NULL) {
        finish = j - 1;
        start_scan = false;
      }
    }

    if (strstr(text[j], "NFQWS2_OPT=") != NULL) {
      if (text[j][len - 2] == '"' && text[j][len - 3] == '=') {
        stand_format = 1;
        start_scan =  true;
        start = j + 1;
      } else if (text[j][len - 2] == '"') {
        stand_format = 0;
        start_scan = true;
        start = j;
      } else printf("Error. Failed to read NFQWS2_OPT correctly");
    }

    if (start_scan && stand_format == 0) {
      bool quotes_opened = false;
      int v = 0;
      for (int l = 0; l < len; l++) {
        if (quotes_opened) {
          nfqws2_opt[0][v] = text[j][l];
          v++;
        }
        if (text[j][l] == '"') {
          quotes_opened = !quotes_opened;
        }
      }
      start_scan = false;
      finish = j;
      nfqws2_opt[0][strlen(nfqws2_opt[0]) - 1] = '\0';
    }
  }
  
  if (stand_format == 1) {
    for (int j = start; j <= finish; j++) {
      strcpy(nfqws2_opt[c], text[j]);
      c++;
    }
  } else if (stand_format == 2) {
    printf("Error. NFQWS2_OPT not found\n");
  }

  info.start = start;
  info.finish = finish;
  info.lines = i;
  info.stand_format = stand_format;
  memcpy(info.text, text, sizeof(text));

  return info;
}

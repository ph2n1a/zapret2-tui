#include "../../include/utils/utils.h"

void collapse_spaces(char *str) {
  if (str == NULL) return;

  int read = 0;
  int write = 0;

  while (str[read] != '\0') {
    if (str[read] != ' ') {
      str[write++] = str[read];
      read++;
      continue;
    }

    int start = read;
    while (str[read] == ' ') {
      read++;
    }

    int spaces = read - start;
    if (spaces == 1) {
      str[write++] = ' ';
    }
  }

  str[write] = '\0';
}

void remove_newlines(char* str) {
  if (str == NULL) return;

  int write = 0;
  for (int read = 0; str[read] != '\0'; read++) {
    if (str[read] != '\n') {
      str[write++] = str[read];
    }
  }
  str[write] = '\0';
}

void copy_file(const char *src, const char *dst) {
    FILE *in = fopen(src, "rb");
    FILE *out = fopen(dst, "wb");
    if (!in || !out) return;

    char buf[8192]; size_t n;
    while ((n = fread(buf, 1, sizeof(buf), in)) > 0)
        fwrite(buf, 1, n, out);

    fclose(in); fclose(out);
}

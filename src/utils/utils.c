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
  while ((n = fread(buf, 1, sizeof(buf), in)) > 0) fwrite(buf, 1, n, out);

  fclose(in); fclose(out);
}

int file_exists_in_dir(const char *dir, const char *filename) {
  if (!dir || !filename) return 0;

  char path[PATH_MAX];
  
  int len = snprintf(path, sizeof(path), "%s/%s", dir, filename);
  if (len < 0 || (size_t)len >= sizeof(path)) {
    return 0;
  }

  struct stat st;
  return stat(path, &st) == 0;
}

int compare_files(const char *path1, const char *path2) {
  FILE *f1 = fopen(path1, "rb");
  FILE *f2 = fopen(path2, "rb");
    
  if (!f1 || !f2) {
    if (f1) fclose(f1);
    if (f2) fclose(f2);
    return -1;
  }

  char buf1[BUFSIZ], buf2[BUFSIZ];
  size_t n1, n2;

  while (1) {
    n1 = fread(buf1, 1, sizeof(buf1), f1);
    n2 = fread(buf2, 1, sizeof(buf2), f2);

    if (n1 != n2) {
      fclose(f1); fclose(f2);
      return 1;
    }

    if (n1 == 0) break;

    if (memcmp(buf1, buf2, n1) != 0) {
      fclose(f1); fclose(f2);
      return 1;
    }
  }

  if (ferror(f1) || ferror(f2)) {
    fclose(f1); fclose(f2);
    return -1;
  }

  fclose(f1); fclose(f2);
  return 0;
}

bool is_root() {
  return geteuid() == 0;
}

#include "../../include/utils/utils.h"

void collapse_spaces(char* str) {
  if (str == NULL) return;

  int write = 0;
  for (int read = 0; str[read] != '\0'; read++) {
    if (str[read] != ' ') {
      str[write++] = str[read];
    } else if (write == 0 || str[write - 1] != ' ') {
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

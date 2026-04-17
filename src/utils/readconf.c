#include "../../include/utils/readconf.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

int handler(void *user, const char *section, const char *name, const char *value) {
  Config *config = (Config *)user;

  if (strcmp(section, "main") == 0) {
    if (strcmp(name, "first_start") == 0) {
      if (strcmp(value, "true") == 0) {
        config->first_start = true;
      } else if (strcmp(value, "false") == 0) {
        config->first_start = false;
      } else {
        printf("Error. Unable to read 'first_start' in src/config.");
      }
    } else if (strcmp(name, "zapret_path") == 0) {
      if (strcmp(value, "none") == 0) {
        strcpy(config->zapret_path, value);
        // printf("Error. You did not specify the path in 'zapret_path'.");
      } else {
        strcpy(config->zapret_path, value);
      }
    } else {
      printf("Error. Not found configs in src/config.");
    }
  } /* else {
    printf("Error. Unable to read src/config.");
  } */

  return 1;
}

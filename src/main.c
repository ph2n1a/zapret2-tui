#include "../include/main.h"

int main() {
  if (!check_dependencies()) {
    printf("check_dependencies returned 1");
    return 1;
  }

  Config config_main;
  if (read_conf_sec(&config_main)) {
    return 1;
  }

  if (config_main.first_start == true) {
    if (first_start()) { return 0; }
  }

  read_conf_engine();

  printf("\nProgram finished!");
  return 0;
}

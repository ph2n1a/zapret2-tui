#include "../include/main.h"

int main() {
  Config config_main;

  ini_parse("./config/config", handler, &config_main);

  printf("first_start: %d\nzapret_path: %s\n", config_main.first_start, config_main.zapret_path);

  return 0;
}

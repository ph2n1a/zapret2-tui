#include "../include/main.h"

int main() {
  if (!check_dependencies()) {
    printf("check_dependencies returned 1");
    return 1;
  }

  Config config_main;
  if (read_conf_sec(&config_main)) {
    printf("read_conf_sec returned 1");
    return 1;
  }

  printf("first_start: %d\nzapret_path: %s\n\n", config_main.first_start, config_main.zapret_path);

  char input1[6];
  char input2[512];

  printf("first_start: ");
  scanf("%5s", input1);
  printf("zapret_path: ");
  scanf("%511s", input2);

  if (write_conf_sec("set_bool", "first_start", input1)) { return 1; }
  if (write_conf_sec("set_str", "zapret_path", input2)) { return 1; }

  if (read_conf_sec(&config_main)) {
    printf("read_conf_sec returned 1");
    return 1;
  }

  printf("\nfirst_start: %d\nzapret_path: %s\n", config_main.first_start, config_main.zapret_path);

  printf("\nProgram finished!");
  return 0;
}

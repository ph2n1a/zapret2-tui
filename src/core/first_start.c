#include "../../include/core/first_start.h"
#include <errno.h>

int first_start() {
  char path[512];
  char choice;

  printf("Welcome to zapret-tui\nThis program is a TUI wrapper for zapret2 only, and to use it, you must have zapret installed. This program was written using zapret2 v0.9.5, but it should work on other versions too. If you don't have zapret2 installed, you can install it at https://github.com/bol-van/zapret2. Enjoy using zapret-tui ;)\n");
  printf("\nSpecify the path to the folder with zapret: ");
  if (scanf("%511s", path) != 1) {
    fprintf(stderr, "Error: failed to read zapret path from stdin.\n");
    return -1;
  }

  if (access(path, F_OK) != 0) {
    fprintf(stderr, "Error: path does not exist: %s\n", path);
    return -1;
  }

  if (write_conf_sec("set_str", "zapret_path", path) != 0) {
    return -1;
  }
  char bin_path[512];

  ssize_t bin_path_len = readlink("/proc/self/exe", bin_path, sizeof(bin_path) - 1);
  if (bin_path_len == -1) {
    fprintf(stderr, "Error: failed to resolve executable path: %s\n", strerror(errno));
    return -1;
  }
  bin_path[bin_path_len] = '\0';
  char *last_slash = strrchr(bin_path, '/');
  if (last_slash) *(last_slash + 1) = '\0';
  
  if (write_conf_sec("set_str", "program_path", bin_path) != 0) {
    return -1;
  }

  printf("\nzapret-tui works on a profile-based basis; one profile = one strategy (including http, tls, and quic). You can quickly switch between strategies, but you'll need to manually enter them in %s/profiles.conf\nDo you want to continue with the program or edit profiles.conf?\n[Y]es/[N]o/[E]dit\n==> ", bin_path);
  if (scanf(" %c", &choice) != 1) {
    fprintf(stderr, "Error: failed to read first-start action from stdin.\n");
    return -1;
  }

  if (write_conf_sec("set_bool", "first_start", "false") != 0) {
    return -1;
  }

  if (choice == 'Y' || choice == 'y') {
    return 0;
  } else if (choice == 'N' || choice == 'n') {
    return 1;
  } else if (choice == 'E' || choice == 'e') {
    const char *editor = getenv("EDITOR");
    if (!editor || !*editor) {
      fprintf(stderr, "Error: EDITOR is not set.\n");
      return -1;
    }
    execlp(editor, editor, "./profiles.conf", NULL);
    fprintf(stderr, "Error: failed to launch editor \"%s\": %s\n", editor, strerror(errno));
    return -1;
  }

  fprintf(stderr, "Error: invalid choice \"%c\". Expected Y, N, or E.\n", choice);
  return -1;
}

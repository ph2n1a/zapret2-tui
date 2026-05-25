#include "../../include/core/create_link.h"

int create_link(char *zapret_path, int *n_profile) {
  char zapret_conf_path[PATH_MAX];
  char zapret_conf_save_path[PATH_MAX];
  char user_conf_path[PATH_MAX];
  
  if (!zapret_path || !n_profile) {
    fprintf(stderr, "Error: create_link received invalid arguments.\n");
    return 1;
  }

  if (snprintf(user_conf_path, sizeof(user_conf_path), "./config/zapret_config/config_%d", *n_profile) >= (int)sizeof(user_conf_path)) {
    fprintf(stderr, "Error: selected config path is too long for profile %d.\n", *n_profile);
    return 1;
  }
  if (snprintf(zapret_conf_path, sizeof(zapret_conf_path), "%s/config", zapret_path) >= (int)sizeof(zapret_conf_path)) {
    fprintf(stderr, "Error: zapret config path is too long.\n");
    return 1;
  }
  if (snprintf(zapret_conf_save_path, sizeof(zapret_conf_save_path), "%s/config.save", zapret_path) >= (int)sizeof(zapret_conf_save_path)) {
    fprintf(stderr, "Error: zapret backup config path is too long.\n");
    return 1;
  }

  if (!file_exists_in_dir(zapret_path, "config.save")) {
    if (rename(zapret_conf_path, zapret_conf_save_path) != 0) {
      fprintf(stderr, "Error: failed to create backup %s: %s\n", zapret_conf_save_path, strerror(errno));
      return 1;
    }
  }

  if (copy_file(user_conf_path, zapret_conf_path) != 0) {
    return 1;
  }

  char buf[32];
  snprintf(buf, sizeof(buf), "%d", *n_profile);
  if (write_conf_sec("set_int", "view_profile", buf) != 0) {
    return 1;
  }

  return 0;
}

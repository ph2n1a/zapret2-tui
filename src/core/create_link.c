#include "../../include/core/create_link.h"

int create_link(char *zapret_path, int *n_profile) {
  char zapret_conf_path[PATH_MAX];
  char zapret_conf_save_path[PATH_MAX];
  char user_conf_path[PATH_MAX];
  
  if (!zapret_path || !n_profile) return 1;

  if (snprintf(user_conf_path, sizeof(user_conf_path), "./config/zapret_config/config_%d", *n_profile) >= (int)sizeof(user_conf_path)) {
    return 1;
  }
  if (snprintf(zapret_conf_path, sizeof(zapret_conf_path), "%s/config", zapret_path) >= (int)sizeof(zapret_conf_path)) {
    return 1;
  }
  if (snprintf(zapret_conf_save_path, sizeof(zapret_conf_save_path), "%s/config.save", zapret_path) >= (int)sizeof(zapret_conf_save_path)) {
    return 1;
  }

  if (!file_exists_in_dir(zapret_path, "config.save")) {
    if (rename(zapret_conf_path, zapret_conf_save_path) != 0) {
      return 1;
    }
  }

  if (copy_file(user_conf_path, zapret_conf_path) != 0) {
    return 1;
  }

  char buf[32];
  snprintf(buf, sizeof(buf), "%d", *n_profile);
  write_conf_sec("set_int", "view_profile", buf);

  return 0;
}

#include "../../include/core/create_link.h"

int create_link(char *zapret_path, int *n_profile) {
  char zapret_conf_path[128];
  char zapret_conf_save_path[128];
  char user_conf_path[128];
  
  snprintf(user_conf_path, sizeof(user_conf_path), "./config/zapret_config/config_%d", *n_profile);
  snprintf(zapret_conf_path, sizeof(zapret_conf_path), "%s/config", zapret_path);
  snprintf(zapret_conf_save_path, sizeof(zapret_conf_save_path), "%s/config.save", zapret_path);

  if (!file_exists_in_dir(zapret_path, "config.save")) rename(zapret_conf_path, zapret_conf_save_path);

  copy_file(user_conf_path, zapret_conf_path);

  char buf[32];
  snprintf(buf, sizeof(buf), "%d", *n_profile);
  write_conf_sec("set_int", "view_profile", buf);

  return 0;
}

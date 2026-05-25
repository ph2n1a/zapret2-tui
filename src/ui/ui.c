#include "../../include/ui/ui.h"
#include <ncurses.h>

WINDOW *header_win = NULL;
WINDOW *main_win = NULL;
WINDOW *menu_win = NULL;

static bool without_sudo;

int max_y, max_x, main_height, main_width, menu_height, menu_width;

void ui_init(bool get_without_sudo) {
  ui_cleanup();

  getmaxyx(stdscr, max_y, max_x);

  if (max_x < 80) max_x = 80;
  if (max_y < 20) max_y = 20;

  main_width = (max_x * 7) / 10;
  if (main_width < 20) main_width = 20;
    
  menu_width = max_x - main_width - 1;
  if (menu_width < 10) menu_width = 10;

  main_height = max_y - 4;
  menu_height = max_y - 4;
  if (main_height < 3) main_height = 3;
  if (menu_height < 3) menu_height = 3;

  header_win = newwin(3, max_x, 0, 0);
  menu_win   = newwin(menu_height, menu_width, 3, 0);
  main_win   = newwin(main_height, main_width, 3, menu_width + 1);

  if (!header_win || !menu_win || !main_win) {
    endwin();
    perror("Failed to create ncurses windows");
    exit(EXIT_FAILURE);
  }

  without_sudo = get_without_sudo;

  init_pair(1, COLOR_GREEN, COLOR_BLACK);
  init_pair(2, COLOR_YELLOW, COLOR_BLACK);
  init_pair(3, COLOR_RED, COLOR_BLACK);
  init_pair(4, COLOR_BLACK, COLOR_WHITE);

  box(header_win, 0, 0);
  box(menu_win, 0, 0);
  box(main_win, 0, 0);

  wrefresh(header_win);
  wrefresh(menu_win);
  wrefresh(main_win);
}


void draw_menu(Profile *profile, int i) {
  mvwprintw(main_win, 2, 2, "id: %d", profile[i].id);
  mvwprintw(main_win, 3, 2, "name: %s", profile[i].name);

  int lines_count = count_word(profile[i].nfqws2_opt);
  char **lines = split_by_new(profile[i].nfqws2_opt, &lines_count);

  short usable_width = main_width - 4;
  int current_row = 6;

  if (usable_width <= 0) { free_split(lines, lines_count); return; }

  mvwprintw(main_win, 4, 2, "NFQWS2_OPT:");

  for (int j = 0; j < lines_count; j++) {
    if (!lines[j]) continue;

    size_t len = strlen(lines[j]);
    for (size_t pos = 0; pos < len; pos += usable_width) {
      if (current_row >= main_height) break;

      size_t chunk_len = (len - pos < usable_width) ? (len - pos) : usable_width;
      if (chunk_len > 1023) chunk_len = 1023;

      char chunk[1024];
      memcpy(chunk, lines[j] + pos, chunk_len);
      chunk[chunk_len] = '\0';

      mvwprintw(main_win, current_row, 2, "%s", chunk);
      current_row++;
    }

    if (j < lines_count - 1 && current_row < main_height) {
      current_row++; 
    }
  }

  free_split(lines, lines_count);
}

void draw_header(int active_code) {
  // 1 - inactive (dead)
  // 0 - active
  // 2 - restart (reload)
  // 3 - error
  // 4 - without sudo

  mvwprintw(header_win, 1, 2, "zapret2 is: ");

  switch (active_code) {
    case 1:
      wattron(header_win, COLOR_PAIR(3));
      mvwprintw(header_win, 1, 14, "ø");
      wattroff(header_win, COLOR_PAIR(3));
      mvwprintw(header_win, 1, 16, " Inactive (dead)");
      break;

    case 0:
      wattron(header_win, COLOR_PAIR(1));
      mvwprintw(header_win, 1, 14, "o");
      wattroff(header_win, COLOR_PAIR(1));
      mvwprintw(header_win, 1, 16, "Active");
      break;

    case 2:
      wattron(header_win, COLOR_PAIR(2));
      mvwprintw(header_win, 1, 14, "¤");
      wattroff(header_win, COLOR_PAIR(2));
      mvwprintw(header_win, 1, 16, " Restart (reload)");
      break;

    case 3: 
      wattron(header_win, COLOR_PAIR(3));
      mvwprintw(header_win, 1, 14, "Error service");
      wattroff(header_win, COLOR_PAIR(3));
      break;

    case 4:
      wattron(header_win, COLOR_PAIR(3));
      mvwprintw(header_win, 1, 14, "¤");
      wattroff(header_win, COLOR_PAIR(3));
      mvwprintw(header_win, 1, 16, " Without SUDO");
      break;
  }

  wattron(header_win, A_REVERSE);
  mvwprintw(header_win, 1, 37, " (h) - help ");
  wattroff(header_win, A_REVERSE);

  mvwprintw(header_win, 1, max_x - 28, "zapret2-tui");

  wattron(header_win, A_REVERSE);
  mvwprintw(header_win, 1, max_x - 15, " v1.0.1 BETA ");
  wattroff(header_win, A_REVERSE);
}

void ui_draw(const AppState *state, Profile *profile, int *view_profile) {
  werase(header_win);
  werase(main_win);
  werase(menu_win);

  int service_status_code = get_service_status();
  if (state->is_reload) service_status_code = 2;
  if (state->service_error) service_status_code = 3;
  if (without_sudo && state->service_error) service_status_code = 4;

  draw_header(service_status_code);

  for (int i = 0; i < state->menu_count; i++) {
    if (i == state->menu_index) {
      int row = 1 + i;
      int text_len = strlen(profile[i].name) + 2;
      int fill_count = menu_width - text_len - 4;

      wattron(menu_win, A_REVERSE);

      if (i == *view_profile) {  
        mvwprintw(menu_win, row, 2, " [ %s ] ", profile[i].name);
        mvwprintw(menu_win, row, 2 + (text_len + 4), "%*s", fill_count - 4, "");
      } else {
        mvwprintw(menu_win, row, 2, " %s ", profile[i].name);
        mvwprintw(menu_win, row, 2 + text_len, "%*s", fill_count, "");
      }

      wattroff(menu_win, A_REVERSE);

      draw_menu(profile, i);
    } else {
      if (i == *view_profile) {
        mvwprintw(menu_win, 1 + i, 2, " [ %s ] ", profile[i].name);
      } else {
        mvwprintw(menu_win, 1 + i, 2, " %s ", profile[i].name);
      }
    }
  }

  if (state->help_window) help_window();
  else clean_between_boxes();

  if (without_sudo) {
    wattron(main_win, A_REVERSE);
    mvwprintw(main_win, main_height - 2, main_width - 17, " WITHOUT SUDO! ");
    wattroff(main_win, A_REVERSE);
  }
  
  box(header_win, 0, 0);
  box(main_win, 0, 0);
  box(menu_win, 0, 0);

  
  wrefresh(header_win);
  wrefresh(main_win);
  wrefresh(menu_win);
}

void help_window() {
  attron(COLOR_PAIR(4));
  mvprintw((max_y / 2) - 6, (max_x / 2) - 20, "                                        ");
  mvprintw((max_y / 2) - 5, (max_x / 2) - 20, " HELP MENU                              ");
  mvprintw((max_y / 2) - 4, (max_x / 2) - 20, " (up) - select profile up               ");
  mvprintw((max_y / 2) - 3, (max_x / 2) - 20, " (down) - select profile down           ");
  mvprintw((max_y / 2) - 2, (max_x / 2) - 20, " (enter) - connect selected profile     ");
  mvprintw((max_y / 2) - 1, (max_x / 2) - 20, "                                        ");
  mvprintw((max_y / 2), (max_x / 2) - 20, " (s) - start service zapret2            ");
  mvprintw((max_y / 2) + 1, (max_x / 2) - 20, " (x) - stop service zapret2             ");
  mvprintw((max_y / 2) + 2, (max_x / 2) - 20, " (r) - reload/restart service zapret2   ");
  mvprintw((max_y / 2) + 3, (max_x / 2) - 20, "                                        ");
  mvprintw((max_y / 2) + 4, (max_x / 2) - 20, " (q) - quit program                     ");
  mvprintw((max_y / 2) + 5, (max_x / 2) - 20, "                                        ");
  attroff(COLOR_PAIR(4));
}

void clean_between_boxes() {
  for (int i = 0; i <= max_x; i++) {
    mvprintw(max_y - 1, i, " ");
  }

  for (int i = 3; i <= max_y; i++) {
    mvprintw(i, menu_width, " ");
  }
}

void ui_cleanup() {
  if (header_win) { delwin(header_win); header_win = NULL; }
  if (main_win)   { delwin(main_win);   main_win = NULL; }
  if (menu_win)   { delwin(menu_win);   menu_win = NULL; }
}

char** split_by_new(const char *text, int *count) {
  if (!text || !count || *count < 0) return NULL;

  short parts = *count + 1;
  char **result = calloc(parts, sizeof(char *));
  if (!result) return NULL;

  char *buf = strdup(text);
  if (!buf) { free(result); return NULL; }

  char *ptr = buf;
  for (int i = 0; i < *count; i++) {
    char *sep = strstr(ptr, "--new");
    if (!sep) break;

    size_t len = sep - ptr;
    result[i] = strndup(ptr, len);
    if (!result[i]) {
      free(buf);
      free_split(result, parts);
      return NULL;
    }
        
    ptr = sep + 5;
    collapse_spaces(result[i]);
  }
  result[*count] = strdup(ptr);
  if (!result[*count]) {
    free(buf);
    free_split(result, parts);
    return NULL;
  }
  collapse_spaces(result[*count]);
  free(buf);

  *count = parts;

  return result;
}

static inline int is_boundary(char c) {
  return !isalnum((unsigned char)c);
}

void free_split(char **arr, int count) {
  if (!arr) return;
  for (int i = 0; i < count; i++) free(arr[i]);
  free(arr);
}

int count_word(const char *text) {
  if (!text || !*text) return 0;

  int count = 0;
  const char *word = "--new";
  const char *pos = text;
  size_t wlen = strlen(word);

  while ((pos = strstr(pos, word)) != NULL) {
    if (pos > text && !is_boundary(pos[-1])) {
      pos += wlen;
      continue;
    } if (pos[wlen] != '\0' && !is_boundary(pos[wlen])) {
      pos += wlen;
      continue;
    }
    count++;
    pos += wlen;
  }
  return count;
}

int get_service_status() {
  const char service_name[16] = "zapret2.service";
  char cmd[256];
  
  snprintf(cmd, sizeof(cmd), "systemctl is-active %s 2>/dev/null", service_name);

  FILE *fp = popen(cmd, "r");
  if (!fp) return -1;

  char status[32] = {0};
  if (fgets(status, sizeof(status), fp) != NULL) {
    status[strcspn(status, "\n\r")] = '\0';
  }

  int close_ret = pclose(fp);
  if (close_ret == -1) return -1;

  if (strcmp(status, "active") == 0) return 0;
    
  return 1;
}

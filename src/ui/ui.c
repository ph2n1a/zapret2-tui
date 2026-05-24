#include "../../include/ui/ui.h"

WINDOW *header_win = NULL;
WINDOW *main_win = NULL;
WINDOW *menu_win = NULL;

int max_y, max_x, main_height, main_width, menu_height, menu_width;

void ui_init() {
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

void ui_draw(const AppState *state, Profile *profile, short *n_profiles, int *view_profile) {
  werase(header_win);
  werase(main_win);
  werase(menu_win);

  box(header_win, 0, 0);
  box(main_win, 0, 0);
  box(menu_win, 0, 0);
    
  for (int i = 0; i < state->menu_count; i++) {
    if (i == state->menu_index) {
      if (i == *view_profile) {
        wattron(menu_win, A_REVERSE);
        mvwprintw(menu_win, 1 + i, 2, " [ %s ] ", profile[i].name);
        wattroff(menu_win, A_REVERSE);
        draw_menu(profile, i);
      } else {
        wattron(menu_win, A_REVERSE);
        mvwprintw(menu_win, 1 + i, 2, " %s ", profile[i].name);
        wattroff(menu_win, A_REVERSE);
        draw_menu(profile, i);
      }
    } else {
      if (i == *view_profile) {
        mvwprintw(menu_win, 1 + i, 2, " [ %s ] ", profile[i].name);
      } else {
        mvwprintw(menu_win, 1 + i, 2, " %s ", profile[i].name);
      }
    }
  }
  
  wrefresh(header_win);
  wrefresh(main_win);
  wrefresh(menu_win);
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
    if (!result[i]) { printf("Error. Handling allocation error"); }
        
    ptr = sep + 5;
    collapse_spaces(result[i]);
  }
  result[*count] = strdup(ptr);
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

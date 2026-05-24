#ifndef INCLUDE_UI_UI_H_
#define INCLUDE_UI_UI_H_

#include <ncurses.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "./core.h"
#include "../utils/readconf.h"
#include "../utils/utils.h"

void ui_init();
void ui_draw(const AppState *state, Profile *profile, short *n_profiles, int *view_profile);
void ui_cleanup();
char** split_by_new(const char *text, int *count);
void free_split(char **arr, int count);
int count_word(const char *text);

#endif  // INCLUDE_UI_UI_H_

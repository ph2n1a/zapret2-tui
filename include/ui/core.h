#ifndef INCLUDE_UI_CORE_H_
#define INCLUDE_UI_CORE_H_

typedef enum {
  INPUT_NONE = 0,
  INPUT_UP,
  INPUT_DOWN,
  INPUT_LEFT,
  INPUT_RIGHT,
  INPUT_ENTER,
  INPUT_QUIT,
  INPUT_REINIT,
} InputAction;

typedef struct {
    int menu_index;
    int menu_count;
    int scrol_opts_index;
    int scrol_opts_count;
    int running;
} AppState;

void core_init(AppState *state, short *n_profiles);
void core_update(AppState *state, InputAction action);

#endif  // INCLUDE_UI_CORE_H_

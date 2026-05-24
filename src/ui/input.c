#include "../../include/ui/input.h"

InputAction input_poll() {
    int ch = getch();
    switch (ch) {
        case KEY_UP: return INPUT_UP;
        case KEY_DOWN: return INPUT_DOWN;
        case KEY_LEFT: return INPUT_LEFT;
        case KEY_RIGHT: return INPUT_RIGHT;
        case '\n':
        case '\r':
        case KEY_ENTER: return INPUT_ENTER;
        case KEY_RESIZE: return INPUT_REINIT;
        case 'q':
        case 27: return INPUT_QUIT;
        default: return INPUT_NONE;
    }
}

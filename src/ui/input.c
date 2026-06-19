#include "../../include/ui/input.h"
#include "../../include/utils/log.h"

InputAction input_poll() {
    int ch = getch();
    switch (ch) {
        case KEY_UP: return INPUT_UP;
        case KEY_DOWN: return INPUT_DOWN;
        case '\n':
        case '\r':
        case KEY_ENTER: return INPUT_ENTER;
        case KEY_RESIZE: return INPUT_REINIT;
        case 'S':
        case 's': return INPUT_START_SERVICE;
        case 'X':
        case 'x': return INPUT_STOP_SERVICE;
        case 'R':
        case 'r': return INPUT_RELOAD_SERVICE;
        case 'H':
        case 'h': return INPUT_OPEN_HELP_WINDOW;
        case 'T':
        case 't': return INPUT_TESTING_ONE;
        case 'A':
        case 'a': return INPUT_TESTING_ALL;
        case 'q':
        case 'Q':
        case 27: return INPUT_QUIT;
        default: return INPUT_NONE;
    }
}

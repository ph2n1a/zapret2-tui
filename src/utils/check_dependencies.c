#include "../../include/utils/check_dependencies.h"
#include <ncurses.h>
#include <confuse.h>

int check_dependencies() {
    printf("✅ ncurses: %s\n", NCURSES_VERSION);
    printf("✅ libconfuse: \n");

    void *handle = dlopen("libconfuse.so", RTLD_LAZY);

    if (!handle) {
        fprintf(stderr,
            "ERROR: libconfuse is not installed or not found.\n"
            "Please install dependencies first.\n"
            "Run: ./dependencies.sh\n"
        );
        return 0;
    }

    dlclose(handle);
    return 1;
}

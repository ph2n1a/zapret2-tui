#include "../../include/utils/check_dependencies.h"
#include <ncurses.h>
#include <confuse.h>

int check_dependencies() {
    printf("\nncurses: %s OK\n", NCURSES_VERSION);

    void *handle = dlopen("libconfuse.so", RTLD_LAZY);

    if (!handle) {
        fprintf(stderr,
            "Error: libconfuse is not installed or could not be loaded.\n"
            "Please install project dependencies first.\n"
            "Run: ./dependencies.sh\n"
        );
        return 0;
    }

    dlclose(handle);

    printf("libconfuse: OK\n\n");
    return 1;
}

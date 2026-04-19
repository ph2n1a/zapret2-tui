#include "../../include/utils/check_dependencies.h"

int check_dependencies() {
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

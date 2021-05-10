#include "util.h"

void util::bail(const char* message, int code) {
    if (code > 0) {
        printError(message);
    } else {
        print(message);
    }

    exit(code);
}

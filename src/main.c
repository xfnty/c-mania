#include <assert.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include <raylib.h>
#include <kvec.h>

#include "util.h"
#include "beatmap.h"


static beatmap_t beatmap;

int main(int argc, const char *argv[]) {
    logging_init();

    if (argc <= 1) {
        printf("Usage: %s <.osz/folder>\n", GetFileName(argv[0]));
        exit(0);
    }

    CHECK_ERROR(beatmap_load(&beatmap, argv[1]));
    // ASSERT(beatmap_load(&beatmap, argv[1]) == ERROR_SUCCESS);
    LOGF_SUCCESS("Loaded \"%s\"", argv[1]);

    return 0;
}

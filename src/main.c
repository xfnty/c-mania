#include "GLFW/glfw3.h"
#include <assert.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include <raylib.h>
#include <kvec.h>

#include <logging.h>
#include <beatmap.h>
#include <string.h>


static beatmap_t        beatmap;
static const int        width = 280;
static const int        height = 480;

// Mainloop
static void init(int argc, const char *argv[]);
static void update();
static void deinit();

// Support functions
void load_beatmap(const char* path);


int main(int argc, const char *argv[]) {
    init(argc, argv);
    deinit();
    return 0;
}

void init(int argc, const char *argv[]) {
    logging_init();

    if (argc <= 1) {
        printf("Usage: %s <.osz/folder>\n", GetFileName(argv[0]));
        exit(0);
    }

    load_beatmap(argv[1]);
}

void deinit() {
    logging_shutdown();
}

void load_beatmap(const char* path) {
    if (!beatmap_load(&beatmap, path))
        exit(-1);
    beatmap_debug_print(&beatmap);
}

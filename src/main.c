#include "GLFW/glfw3.h"
#include "defines.h"
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
static int              difficulty_i;
static const int        width = 280;
static const int        height = 480;

static float judgement_line_y = 0;


// Mainloop
static void init(int argc, const char *argv[]);
// static void update();
// static void update_difficulty_changer();
// static void update_judgement_line_y();
// static void draw_timing_points();
static void deinit();

// Support functions
static void load_beatmap(const char* path);
// static void draw_info();

// static float playfield_y_to_window_y(float y);

int main(int argc, const char *argv[]) {
    init(argc, argv);

    // while (!WindowShouldClose()) {
    //     BeginDrawing();
    //     ClearBackground(WHITE);
    //     update();
    //     draw_info();
    //     EndDrawing();
    // }

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

    // InitWindow(width, height, "CMania");
    // SetTargetFPS(60);
}

// void update() {
//     update_difficulty_changer();
//     update_judgement_line_y();
//     draw_timing_points();
// }

void deinit() {
    // CloseWindow();
    logging_shutdown();
}

void load_beatmap(const char* path) {
    if (!beatmap_load(&beatmap, path))
        exit(-1);
    beatmap_debug_print(&beatmap);

    if (kv_size(beatmap.difficulties) == 0) {
        LOG("No difficulties to play");
        exit(0);
    }

    difficulty_i = 0;
}

// void draw_info() {
//     DrawText(TextFormat("[%d] %s", difficulty_i, kv_A(beatmap.difficulties, difficulty_i).name), 0, 0, 16, BLACK);
//     DrawText(TextFormat("pos: %.0f", judgement_line_y), 0, 18, 16, GRAY);
// }

// void update_difficulty_changer() {
//     if (IsKeyPressed(KEY_RIGHT)) {
//         difficulty_i += 1;
//         judgement_line_y = 0;
//     }

//     difficulty_i %= kv_size(beatmap.difficulties);
// }

// void update_judgement_line_y() {
//     judgement_line_y += GetMouseWheelMove() * 100;
// }

// void draw_timing_points() {
//     playfield_t* playfield = &kv_A(beatmap.difficulties, difficulty_i).playfield;

//     for (int i = 0; i < kv_size(playfield->timing_point_Ys); i++) {
//         float pf_y = kv_A(playfield->timing_point_Ys, i);
//         float y = playfield_y_to_window_y(pf_y);
//         DrawLine(0, y, width, y, GREEN);
//         DrawText(TextFormat("%.1f", pf_y), 2, y - 12, 10, GREEN);
//     }

//     DrawLine(0, playfield_y_to_window_y(0), width, playfield_y_to_window_y(0), BLACK);
// }

// float playfield_y_to_window_y(float y) {
//     return -y + judgement_line_y + height / 2.0f;
// }

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


// Variables
static const int    width = 280;
static const int    height = 480;
static beatmap_t    beatmap;
static int          difficulty_i;
static float        judgement_line_y = 0;

// Mainloop
static void init(int argc, const char *argv[]);
static void update();
static void update_difficulty_changer();
static void update_judgement_line_y();
static void draw_timing_points();
static void deinit();

// Support functions
static void load_beatmap(const char* path);
static void draw_info();
static float playfield_y_to_window_y(float y);


int main(int argc, const char *argv[]) {
    init(argc, argv);

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(WHITE);
        update();
        EndDrawing();
    }

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

    InitWindow(width, height, "CMania");
    SetTargetFPS(60);
}

void update() {
    update_difficulty_changer();
    update_judgement_line_y();
    draw_timing_points();
    draw_info();
}

void deinit() {
    CloseWindow();
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

void draw_info() {
    DrawText(TextFormat("[%d] %s", difficulty_i, kv_A(beatmap.difficulties, difficulty_i).name), 0, 0, 16, BLACK);
    DrawText(TextFormat("pos: %.0f", judgement_line_y), 0, 18, 16, GRAY);
}

void update_difficulty_changer() {
    if (IsKeyPressed(KEY_RIGHT)) {
        difficulty_i += 1;
        judgement_line_y = 0;
    }

    difficulty_i %= kv_size(beatmap.difficulties);
}

void update_judgement_line_y() {
    judgement_line_y += GetMouseWheelMove() * 100;
}

void draw_timing_points() {
    difficulty_t* d = &kv_A(beatmap.difficulties, difficulty_i);

    for (int i = 0; i < kv_size(d->timing_points); i++) {
        timing_point_t* tm = &kv_A(d->timing_points, i);
        float y = playfield_y_to_window_y(tm->y);
        DrawLine(0, y, width, y, GREEN);
        DrawText(TextFormat("%.1f", tm->y), 4, y - 12, 12, GREEN);
        DrawText(TextFormat("%.1f %.1f", tm->BPM, tm->SV), width - 50, y - 12, 12, GREEN);
    }

    DrawLine(0, playfield_y_to_window_y(0), width, playfield_y_to_window_y(0), BLACK);
}

float playfield_y_to_window_y(float y) {
    y = -y + judgement_line_y;
    return (y / 480.0f * height) + height / 2.0f;
}

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
static int          width = 280;
static int          height = 240;
static beatmap_t    beatmap;
static int          difficulty_i;
static float        judgement_line_y = 0;
static int          timing_point_render_range_start = 0;
static int          timing_point_render_range_end = 0;
static float        playfield_to_window_scale = 1;

// Mainloop
static void init(int argc, const char *argv[]);
static void update();
static void update_difficulty_changer();
static void update_judgement_line_y();
static void update_timing_point_render_range();
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
    SetWindowState(FLAG_WINDOW_RESIZABLE);
    SetTargetFPS(60);

    timing_point_render_range_end = kv_size(kv_A(beatmap.difficulties, difficulty_i).timing_points);
}

void update() {
    update_difficulty_changer();
    update_judgement_line_y();
    update_timing_point_render_range();
    draw_timing_points();
    draw_info();

    width = GetScreenWidth();
    height = GetScreenHeight();
    playfield_to_window_scale = height / 480.0f;
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
    // DrawText(TextFormat("range: %d-%d (%d)", timing_point_render_range_start, timing_point_render_range_end, timing_point_render_range_end - timing_point_render_range_start), 0, 36, 16, GRAY);
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

void update_timing_point_render_range() {
    float upper_y = judgement_line_y + (height / 2.0f / playfield_to_window_scale);
    float lower_y = judgement_line_y - (height / 2.0f / playfield_to_window_scale);

    difficulty_t* d = &kv_A(beatmap.difficulties, difficulty_i);
    for (int i = 0; i < kv_size(d->timing_points); i++) {
        timing_point_t* tm = &kv_A(d->timing_points, i);

        timing_point_render_range_start = (tm->y < lower_y) ? i : timing_point_render_range_start;
        // NOTE: inclusive end
        timing_point_render_range_end = (tm->y < upper_y) ? i : timing_point_render_range_end;
    }
}

void draw_timing_points() {
    difficulty_t* d = &kv_A(beatmap.difficulties, difficulty_i);

    int i = timing_point_render_range_start;
    for (; i < MIN(timing_point_render_range_end + 1, kv_size(d->timing_points)); i++) {
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
    return (y * playfield_to_window_scale) + height / 2.0f;
}

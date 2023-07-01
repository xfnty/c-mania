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

#undef LOGF
#undef LOG
#define LOGF(fmt, ...) printf(fmt "\n", __VA_ARGS__);
#define LOG(msg) printf(msg "\n");


// Variables
static const float judgement_line_window_offset = 0.8f;

static int width = 280;
static int height = 480;

static beatmap_t        beatmap;
static difficulty_t*    difficulty;

static seconds_t        judgement_line_time_pos = 0;


// Mainloop
static void init(int argc, const char *argv[]);
static void update();
static void deinit();


// Support functions
static void load_beatmap(const char* path);
static void draw_judgement_line();
static void draw_info();

static float get_playfield_y_for_time(seconds_t time);
static float get_playfield_y_for_window_y(float y);
static float get_window_y_for_playfield_y(float y);


int main(int argc, const char *argv[]) {
    init(argc, argv);

    while (!WindowShouldClose()) {
        BeginDrawing();
            ClearBackground(WHITE);
            update();
        EndDrawing();

        width = GetScreenWidth();
        height = GetScreenHeight();
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
}

void update() {
    draw_judgement_line();
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

    difficulty = &kv_A(beatmap.difficulties, 0);
}

void draw_info() {
    DrawFPS(0, 0);
    DrawText(TextFormat("%s", difficulty->name), 0, 20, 16, BLACK);
    DrawText(TextFormat("pos: %.1f", judgement_line_time_pos), 0, 38, 16, GRAY);
}

void draw_judgement_line() {
    DrawLine(0, height * judgement_line_window_offset, width, height * judgement_line_window_offset, GRAY);
}

void update_judgement_line_y() {
}

float get_playfield_y_for_time(seconds_t time) {
    timing_point_t* tm = difficulty_get_timing_point_for_time(difficulty, time);
    if (tm == NULL) {
        LOGF("Could not find timing point for time %f", time);
        return 0;
    }

    return tm->y + 100 * tm->SV * (time - tm->time) / (60 / tm->BPM);
}

float get_playfield_y_for_window_y(float y) {
    float judgement_line_y = get_playfield_y_for_time(judgement_line_time_pos);
    float judgement_line_window_y = height * judgement_line_window_offset;

    return judgement_line_y * (height / 480.0f) + judgement_line_window_y;
}

float get_window_y_for_playfield_y(float y) {
    float judgement_line_y = get_playfield_y_for_time(judgement_line_time_pos);
    float judgement_line_window_y = height * judgement_line_window_offset;
}

// void draw_timing_points() {
//     difficulty_t* d = &kv_A(beatmap.difficulties, difficulty_i);

//     int i = timing_point_render_range[0];
//     for (; i < MIN(timing_point_render_range[1] + 1, kv_size(d->timing_points)); i++) {
//         timing_point_t* tm = &kv_A(d->timing_points, i);

//         float y = playfield_y_to_window_y(tm->y);

//         DrawLine(0, y, width, y, GREEN);
//         DrawText(TextFormat("%.1f", tm->y), 4, y - 12, 12, GREEN);
//         DrawText(TextFormat("%.1f %.1f", tm->BPM, tm->SV), width - 50, y - 12, 12, GREEN);
//     }

//     DrawLine(0, playfield_y_to_window_y(0), width, playfield_y_to_window_y(0), BLACK);
// }


// float playfield_y_to_window_y(float y) {
//     y = -y + judgement_line_y;
//     return (y * playfield_to_window_scale) + height / 2.0f;
// }

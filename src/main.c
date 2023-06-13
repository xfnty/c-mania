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
static int          height = 480;
static beatmap_t    beatmap;
static int          difficulty_i;
static float        judgement_line_y = 0;
static int          timing_point_render_range[2] = {0};
static int          hitobject_render_range[2] = {0};
static float        playfield_to_window_scale = 1;
static bool         autoplayer_enabled = false;
static float        autoplayer_judgement_line_speed = 0;
static Music        audio;
static float        audio_pos;

// Mainloop
static void init(int argc, const char *argv[]);
static void update();
static void update_difficulty_changer();
static void update_judgement_line_y();
static void update_timing_point_render_range();
static void update_hitobject_render_range();
static void update_autoplayer();
static void draw_timing_points();
static void draw_hitobjects();
static void draw_judgement_line();
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
    // SetTargetFPS(60);

    difficulty_t* d = &kv_A(beatmap.difficulties, difficulty_i);

    timing_point_render_range[1]    = kv_size(d->timing_points);
    hitobject_render_range[1]       = kv_size(d->hitobjects);
}

void update() {
    update_difficulty_changer();
    update_judgement_line_y();
    update_timing_point_render_range();
    update_hitobject_render_range();
    update_autoplayer();

    draw_hitobjects();
    draw_timing_points();
    draw_info();
    draw_judgement_line();

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
    DrawFPS(0, 0);
    DrawText(TextFormat("[%d] %s", difficulty_i, kv_A(beatmap.difficulties, difficulty_i).name), 0, 20, 16, BLACK);
    DrawText(TextFormat("pos: %.0f", judgement_line_y), 0, 38, 16, GRAY);
    DrawText(TextFormat("auto: %d spd=%.3f", autoplayer_enabled, autoplayer_judgement_line_speed), 0, 56, 16, (autoplayer_enabled) ? GREEN : GRAY);
}

void draw_judgement_line() {
    DrawLine(0, height / 2.0f, width, height / 2.0f, GRAY);
}

void update_difficulty_changer() {
    if (IsKeyPressed(KEY_RIGHT)) {
        difficulty_i += 1;
        judgement_line_y = 0;
        timing_point_render_range[0] = 0;
        timing_point_render_range[1] = 0;
        hitobject_render_range[0] = 0;
        hitobject_render_range[1] = 0;
        autoplayer_enabled = false;
    }

    difficulty_i %= kv_size(beatmap.difficulties);
}

void update_judgement_line_y() {
    if (!autoplayer_enabled)
        judgement_line_y += GetMouseWheelMove() * (IsKeyDown(KEY_LEFT_SHIFT) ? 1000 : 100);
}

void update_timing_point_render_range() {
    float upper_y = judgement_line_y + (height / 2.0f / playfield_to_window_scale);
    float lower_y = judgement_line_y - (height / 2.0f / playfield_to_window_scale);

    difficulty_t* d = &kv_A(beatmap.difficulties, difficulty_i);
    for (int i = 0; i < kv_size(d->timing_points); i++) {
        timing_point_t* tm = &kv_A(d->timing_points, i);

        timing_point_render_range[0] = (tm->y < lower_y) ? i : timing_point_render_range[0];
        // NOTE: inclusive end
        timing_point_render_range[1] = (tm->y < upper_y) ? i : timing_point_render_range[1];
    }
}

void update_hitobject_render_range() {
    float upper_y = judgement_line_y + (height / 2.0f / playfield_to_window_scale);
    float lower_y = judgement_line_y - (height / 2.0f / playfield_to_window_scale);

    difficulty_t* d = &kv_A(beatmap.difficulties, difficulty_i);
    for (int i = 0; i < kv_size(d->hitobjects); i++) {
        hitobject_t* ho = &kv_A(d->hitobjects, i);

        hitobject_render_range[0] = (ho->start_y < lower_y) ? i : hitobject_render_range[0];
        hitobject_render_range[1] = (ho->start_y < upper_y) ? i : hitobject_render_range[1];
    }
}

void update_autoplayer() {
    if (IsKeyPressed(KEY_SPACE)) {
        autoplayer_enabled = !autoplayer_enabled;
    }

    autoplayer_judgement_line_speed = 0;

    if (!autoplayer_enabled)
        return;

    difficulty_t* d = &kv_A(beatmap.difficulties, difficulty_i);
    timing_point_t* atm = NULL;
    int i = MAX(0, timing_point_render_range[0] - 1);
    for (; i < MIN(timing_point_render_range[1] + 1, kv_size(d->timing_points)); i++) {
        timing_point_t* tm = &kv_A(d->timing_points, i);
        if (tm->y <= judgement_line_y)
            atm = tm;
        else
            break;
    }

    if (atm == NULL) {
        LOG("Could not find a timing point associated with judgement line's Y");
        autoplayer_enabled = false;
        return;
    }

    autoplayer_judgement_line_speed = 100 * atm->SV * 1 / (60.0f / atm->BPM);
    judgement_line_y += autoplayer_judgement_line_speed * GetFrameTime();
}

void draw_timing_points() {
    difficulty_t* d = &kv_A(beatmap.difficulties, difficulty_i);

    int i = timing_point_render_range[0];
    for (; i < MIN(timing_point_render_range[1] + 1, kv_size(d->timing_points)); i++) {
        timing_point_t* tm = &kv_A(d->timing_points, i);

        float y = playfield_y_to_window_y(tm->y);

        DrawLine(0, y, width, y, GREEN);
        DrawText(TextFormat("%.1f", tm->y), 4, y - 12, 12, GREEN);
        DrawText(TextFormat("%.1f %.1f", tm->BPM, tm->SV), width - 50, y - 12, 12, GREEN);
    }

    DrawLine(0, playfield_y_to_window_y(0), width, playfield_y_to_window_y(0), BLACK);
}

void draw_hitobjects() {
    difficulty_t* d = &kv_A(beatmap.difficulties, difficulty_i);

    int i = hitobject_render_range[0];
    for (; i < MIN(hitobject_render_range[1] + 1, kv_size(d->hitobjects)); i++) {
        hitobject_t* ho = &kv_A(d->hitobjects, i);

        float y = playfield_y_to_window_y(ho->start_y);

        DrawRectangle((width / d->CS * ho->column), y - 2, width / d->CS, 3, RED);
    }

    DrawLine(0, playfield_y_to_window_y(0), width, playfield_y_to_window_y(0), BLACK);
}

float playfield_y_to_window_y(float y) {
    y = -y + judgement_line_y;
    return (y * playfield_to_window_scale) + height / 2.0f;
}

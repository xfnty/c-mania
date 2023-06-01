#include <assert.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include <raylib.h>
#include <kvec.h>

#include <logging.h>
#include <beatmap.h>
#include <string.h>


/*  ##### Slider Velocity, BPM and other things #####
 *
 *
 *  Glossary:
 *      ...(?)              - Things I'm unsure(?) about.
 *
 *      delta               - Time spent processing previous frame.
 *
 *      Osu! pixels, opx    - A pixel on a 640x480 field.
 *
 *      Regular beat        - A single beat in 3/4 or 4/4 scale.
 *      Full beat           - Consists of 3 or 4 regular beats.
 *
 *      Note, nt            - A hit object.
 *      nt.time             - Note's appear time in seconds.
 *      nt.endTime          - End of a hold note.
 *
 *      Timing point, tp    - A Timing Point.
 *      atp, ptp            - Active (current) and previous timing points.
 *      tp.time             - Start time of a timing point.
 *      tp.meter            - A number of regular beats in a full beat. May be
 *                            only 3 (3/4) or 4 (4/4).
 *      tp.bl               - Length in seconds of a regular beat.
 *      tp.fbl              - Length of a full beat (tp.bl * tp.meter).
 *      tp.SV               - The amount of hundreds of osu pixels an object
 *                            would travel in one full beat. [1]
 *
 *      Playfield, pf       - The playfield on which the notes are placed.
 *      pf.vh               - Height of the visible part of the playfield which
 *                            is always 480 (opx).
 *      ...Y                - The absolute Y coordinate (opx) of an object on the
 *                            playfield.
 *
 *      Judgement Line, jl  - Acts like a cursor on a stationary playfield.
 *
 *  Links:
 *      [1]: https://osu.ppy.sh/wiki/en/Gameplay/Hit_object/Slider/Slider_velocity
 *
 *  Notes:
 *      In osu client hit objects does not move relative to each other
 *      but the field itself moves with different speeds.
 *
 *      Distance (opx) traveled by the judgement line between frames:
 *      d = tp.SV * 100 * (delta / tp.fbl)
 *
 *      Each notes's absolute or playfield Y position can be described in this way:
 *      ntY = atpY + atp.SV * 100 * (nt.time - atp.time) / atp.fbl
 *
 *      atpY = ptpY + ptp.SV * 100 * (atp.time - ptp.time) / ptp.fbl
 */


typedef enum {
    NOTE_CLICK,
    NOTE_HOLD_START,
    NOTE_HOLD_END,
} event_type_t;

typedef struct note_event_s {
    event_type_t type;
    float time;
} note_event_t;

typedef kvec_t(note_event_t) column_t;

static kvec_t(column_t) columns;
static Sound hit;
static Music audio;
static beatmap_t beatmap;
static const int width = 280;
static const int height = 480;
static const float line_y = height * 0.9f;

static void init(int argc, const char *argv[]);
static void deinit();
static void load_beatmap(const char* filepath);
static void load_note_columns();
static void draw_notes();
static void draw_keys();
static void draw_info();
static void update_input();
static void update_autoplayer();
static void update_difficulty();
static void update_events();


static float    bpm = -1;
static int      last_event = -1;
static int      last_timing_point = -1;
static int      last_hit_col_i[9] = {-1};
static int      hit_note_count = 0;
static float    time_window = 1;
static float    vol = 0.05;
static double   hit_anims[9] = {-10};
static float    pos = 0;
int main(int argc, const char *argv[]) {
    init(argc, argv);

    LOG("playing");
    while (!WindowShouldClose()) {
        pos = GetMusicTimePlayed(audio);

        BeginDrawing();
        ClearBackground(WHITE);

        DrawLine(0, line_y, width, line_y, LIGHTGRAY);

        draw_notes();
        draw_keys();
        draw_info();

        UpdateMusicStream(audio);
        update_input();
        update_autoplayer();
        update_difficulty();

        EndDrawing();
    }

    deinit();
    return 0;
}


void init(int argc, const char *argv[]) {
    logging_init();

    if (argc <= 1) {
        printf("Usage: %s <.osu file>\n", GetFileName(argv[0]));
        exit(0);
    }

    ChangeDirectory(GetDirectoryPath(argv[0]));

    InitAudioDevice();
    if (!IsAudioDeviceReady()) {
        LOG("Failed to initialize audio");
        exit(-1);
    }

    hit = LoadSound("./assets/hit2.wav");
    if (!IsSoundReady(hit)) {
        LOG("Could not load hit sound");
        exit(-1);
    }
    SetSoundVolume(hit, 0.75f);

    load_beatmap(argv[1]);
    load_note_columns();

    audio = LoadMusicStream(beatmap.audio_filename);
    if (!IsMusicReady(audio)) {
        LOG("Failed to load audio");
        exit(-1);
    }
    SetMusicVolume(audio, 1);

    PlayMusicStream(audio);
    SetMasterVolume(vol);

    InitWindow(width, height, "CMania");
    // SetTargetFPS(60);
}

void deinit() {
    CloseAudioDevice();
    CloseWindow();
    logging_shutdown();
}

void draw_notes() {
    for (int ci = 0; ci < kv_size(columns); ci++) {
        column_t* col = &kv_A(columns, ci);

        for (int i = last_hit_col_i[ci] + 1; i < kv_size(*col); i++) {
            note_event_t* event = &kv_A(*col, i);

            if (i > last_hit_col_i[ci] + 1 && event->time >= pos + time_window)
                break;

            if (event->type == NOTE_CLICK) {
                float current_y = line_y + (pos - event->time) * height;
                DrawRectangle(
                    10 + (width - 20) / beatmap.CS * ci,
                    current_y - 10,
                    (width - 20) / beatmap.CS,
                    10,
                    RED
                );
            }
            else if (event->type == NOTE_HOLD_START && i + 1 < kv_size(*col)) {
                note_event_t* next_event = &kv_A(*col, i + 1);

                float current_event_y = line_y + (pos - event->time) * height;
                float next_event_y = line_y + (pos - next_event->time) * height;

                DrawRectangle(
                    10 + (width - 20) / beatmap.CS * ci,
                    next_event_y,
                    (width - 20) / beatmap.CS,
                    current_event_y - next_event_y,
                    RED
                );
            }
            else if (event->type == NOTE_HOLD_END && i == last_hit_col_i[ci] + 1) {
                float current_y = line_y + (pos - event->time) * height;

                DrawRectangle(
                    10 + (width - 20) / beatmap.CS * ci,
                    current_y,
                    (width - 20) / beatmap.CS,
                    line_y - current_y,
                    RED
                );
            }
        }
    }
}

void update_autoplayer() {
    for (int ci = 0; ci < kv_size(columns); ci++) {
        column_t* col = &kv_A(columns, ci);
        note_event_t* event = &kv_A(*col, last_hit_col_i[ci] + 1);

        if (event->type == NOTE_CLICK) {
            if (event->time <= pos) {
                last_hit_col_i[ci]++;
                hit_anims[ci] = GetTime();
                hit_note_count++;
                PlaySound(hit);
            }
        }
        else if (event->type == NOTE_HOLD_START) {
            if (event->time <= pos) {
                last_hit_col_i[ci]++;
                PlaySound(hit);
            }
        }
        else if (event->type == NOTE_HOLD_END) {
            hit_anims[ci] = GetTime();
            if (event->time <= pos) {
                last_hit_col_i[ci]++;
                hit_note_count++;
                PlaySound(hit);
            }
        }

        hit_note_count = max(hit_note_count, last_hit_col_i[ci]);
    }
}

void update_difficulty() {
    if (last_timing_point + 1 >= kv_size(beatmap.timing_points))
        return;

    beatmap_timing_point_t* tm = &kv_A(beatmap.timing_points, last_timing_point + 1);
    if (tm->time_start / 1000.0f <= pos) {
        if (tm->is_uninherited)
            bpm = 1.0f / tm->length * 60000;
        LOGF(
            "Timing Point: [%s] BPM: %7.0f, SV: %5.1f, Meter: %d",
            (tm->is_uninherited) ? "!" : "+",
            bpm,
            (tm->is_uninherited) ? (beatmap.SV) : (beatmap.SV * (-tm->length / 100.0f)),
            tm->meter
        );
        last_timing_point++;
    }
}

void draw_keys() {
    for (int i = 0; i < beatmap.CS; i++) {
        float opacity = 1 - min((GetTime() - hit_anims[i]) / 0.2f, 1);

        DrawRectangle(
            10 + (width - 20) / beatmap.CS * i,
            line_y,
            (width - 20) / beatmap.CS,
            height * 0.2f,
            Fade(BLACK, 0.75f * opacity)
        );
    }
}

void draw_info() {
    beatmap_timing_point_t* tm = &kv_A(beatmap.timing_points, last_timing_point);
    DrawFPS(0, 0);
    DrawText(TextFormat("vol %.2f", vol), 0, 21, 16, ORANGE);
    DrawText(TextFormat("Note %d/%d", hit_note_count, kv_size(beatmap.notes)), 0, 38, 16, RED);
    DrawText(TextFormat("BPM %.0f", bpm), 0, 54, 16, BLACK);
    DrawText(
        TextFormat(
            "SV %.1f",
            (tm->is_uninherited) ? (beatmap.SV) : (beatmap.SV * (-tm->length / 100.0f))
        ),
        0,
        70,
        16,
        DARKGRAY
    );
}

void load_note_columns() {
    LOG("preprocessing hit objects ...");
    kv_init(columns);
    kv_resize(column_t, columns, beatmap.CS);
    for (int i = 0; i < beatmap.CS; i++) {
        column_t col;
        kv_init(col);
        kv_push(column_t, columns, col);
    }
    for (int i = 0; i < kv_size(beatmap.notes); i++) {
        beatmap_note_t note = kv_A(beatmap.notes, i);
        column_t* column = &kv_A(columns, note.column);

        if (note.is_hold_note) {
            note_event_t hold_start = { NOTE_HOLD_START, note.time_start / 1000.0f };
            note_event_t hold_end = { NOTE_HOLD_END, note.time_end / 1000.0f };
            kv_push(note_event_t, *column, hold_start);
            kv_push(note_event_t, *column, hold_end);
        }
        else {
            note_event_t event = { NOTE_CLICK, note.time_start / 1000.0f };
            kv_push(note_event_t, *column, event);
        }
    }
    // for (int i = 0; i < kv_size(columns); i++) {
    //     LOGF("Column %d: %d notes", i + 1, kv_size(kv_A(columns, i)));
    // }
}

void load_beatmap(const char* filepath) {
    if (!beatmap_load(filepath, &beatmap, false))
        exit(-1);
    beatmap_debug_print(&beatmap);

    ChangeDirectory(GetDirectoryPath(filepath));
    if (!FileExists(beatmap.audio_filename)) {
        LOGF("File \"%s\" does not exists", beatmap.audio_filename);
        exit(-1);
    }
}

void update_input() {
    if (IsKeyPressed(KEY_SPACE)) {
        if (IsMusicStreamPlaying(audio))
            PauseMusicStream(audio);
        else
            ResumeMusicStream(audio);
    }
    float wh = GetMouseWheelMove();
    if (wh != 0) {
        vol = max(0, min(1, vol + wh * 0.05f));
        SetMasterVolume(vol);
    }
    // if (IsKeyPressed(KEY_ENTER)) {
    //     hit_note_count = 0;
    //     for (int i = 0; i < beatmap.CS; i++)
    //         last_hit_col_i[i] = -1;
    //     SeekMusicStream(audio, 0);
    // }
    // if (IsKeyPressed(KEY_LEFT)) {
    //     hit_note_count = 0;
    //     for (int i = 0; i < beatmap.CS; i++)
    //         last_hit_col_i[i] = -1;
    //     SeekMusicStream(audio, max(0, pos - 5));
    // }
    if (IsKeyPressed(KEY_RIGHT)) {
        SeekMusicStream(audio, min(pos + 5, GetMusicTimeLength(audio)));
    }
}

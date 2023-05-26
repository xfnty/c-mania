#include <assert.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include <raylib.h>
#include <kvec.h>

#include <logging.h>
#include <beatmap.h>
#include <string.h>


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


static int      last_hit_col_i[9] = {-1};
static int      hit_note_count = 0;
static float    time_window = 1;
static int      last_hit_i = -1;
static float    vol = 0.3;
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

    InitWindow(width, height, "CMania");
    // SetTargetFPS(60);

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
    SetSoundVolume(hit, 1);

    load_beatmap(argv[1]);
    load_note_columns();

    audio = LoadMusicStream(beatmap.audio_filename);
    if (!IsMusicReady(audio)) {
        LOG("Failed to load audio");
        exit(-1);
    }
    SetMusicVolume(audio, 0.75f);

    PlayMusicStream(audio);
    SetMasterVolume(0.1);
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

            if (event->time >= pos + time_window)
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
                hit_note_count++;
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
    DrawFPS(0, 0);
    DrawText(TextFormat("vol %.2f", vol), 0, 21, 16, ORANGE);
    DrawText(TextFormat("Note %d/%d", hit_note_count, kv_size(beatmap.notes)), 0, 38, 16, RED);
    DrawText(TextFormat("Pos %2.f", pos), 0, 54, 16, GRAY);

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

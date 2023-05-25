#include <assert.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include <raylib.h>
#include <kvec.h>

#include <logging.h>
#include <beatmap.h>
#include <string.h>


int main(int argc, char const *argv[]) {
    logging_init();

    if (argc <= 1) {
        printf("Usage: %s <.osu file>\n", GetFileName(argv[0]));
        return 0;
    }

    ChangeDirectory(GetDirectoryPath(argv[0]));

    const int width = 280;
    const int height = 480;
    InitWindow(width, height, "CMania");
    // SetTargetFPS(60);

    InitAudioDevice();
    if (!IsAudioDeviceReady()) {
        LOG("Failed to initialize audio");
        return -1;
    }

    Sound hit = LoadSound("./assets/hit2.wav");
    SetSoundVolume(hit, 0.5);

    const char* beatmap_path = argv[1];

    beatmap_t beatmap;
    if (!beatmap_load(beatmap_path, &beatmap, false))
        return -1;
    beatmap_debug_print(&beatmap);

    ChangeDirectory(GetDirectoryPath(beatmap_path));
    if (!FileExists(beatmap.audio_filename)) {
        LOGF("File \"%s\" does not exists", beatmap.audio_filename);
        return -1;
    }


    Music audio = LoadMusicStream(beatmap.audio_filename);
    if (!IsMusicReady(audio)) {
        LOG("Failed to load audio");
        return -1;
    }
    SetMusicVolume(audio, 1);

    PlayMusicStream(audio);
    SetMasterVolume(0.1);

    float time_window = 1;
    int last_hit_i = -1;
    float vol = 0.3;
    double hit_anims[9] = {-10};
    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(WHITE);

        DrawLine(0, height * 0.8f, width, height * 0.8f, LIGHTGRAY);
        float pos = GetMusicTimePlayed(audio);

        for (int i = last_hit_i + 1; i < kv_size(beatmap.notes); i++) {
            beatmap_note_t note = kv_A(beatmap.notes, i);
            float note_time = note.time_start / 1000.0f;

            if (note_time < pos - 0.2f)
                continue;
            if (note_time > pos + time_window)
                break;

            float y_start = height * 0.8f + (pos - note_time) * height;
            float note_height = (note.is_hold_note)
                ? y_start - (height * 0.8f + (pos - note.time_end / 1000.0f) * height)
                : (10);
            DrawRectangle(
                10 + (width - 20) / beatmap.CS * note.column,
                y_start - note_height,
                (width - 20) / beatmap.CS,
                note_height,
                (last_hit_i < i) ? RED : GREEN
            );

            if (note_time <= pos && last_hit_i < i) {
                last_hit_i = i;
                hit_anims[note.column] = GetTime();
                PlaySound(hit);
            }
        }

        for (int i = 0; i < beatmap.CS; i++) {
            float opacity = 1 - min((GetTime() - hit_anims[i]) / 0.2f, 1);

            DrawRectangle(
                10 + (width - 20) / beatmap.CS * i,
                height * 0.8f,
                (width - 20) / beatmap.CS,
                height * 0.2f,
                Fade(RED, 0.5 * opacity)
            );
        }

        DrawFPS(0, 0);
        DrawText(TextFormat("vol %.2f", vol), 0, 21, 16, ORANGE);
        DrawText(TextFormat("Note %d/%d", last_hit_i, kv_size(beatmap.notes)), 0, 38, 16, RED);
        EndDrawing();

        UpdateMusicStream(audio);
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
        if (IsKeyPressed(KEY_ENTER)) {
            last_hit_i = -1;
            SeekMusicStream(audio, 0);
        }
        if (IsKeyPressed(KEY_LEFT)) {
            last_hit_i = -1;
            SeekMusicStream(audio, max(0, pos - 5));
        }
        if (IsKeyPressed(KEY_RIGHT)) {
            SeekMusicStream(audio, min(pos + 5, GetMusicTimeLength(audio)));
        }
    }

    CloseAudioDevice();
    CloseWindow();
    logging_shutdown();
    return 0;
}

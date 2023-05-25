#include "kvec.h"
#include <beatmap.h>

#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <raylib.h>
#include <raymath.h>
#include <khash.h>

#include <defines.h>
#include <logging.h>
#include <utils.h>

#define STRCP(dest, src)\
    memcpy((void*)(dest), src, min(strlen(src), STACKARRAY_SIZE(dest) - 1));\
    (dest)[STACKARRAY_SIZE(dest) - 1] = '\0';

// TODO: use scanf()
// TODO: track memory allocations

typedef struct cursor_s {
    const char* start;
    const char* end;
    int len;
    const char* current;
} cursor_t;

bool cursor_get_next_line(cursor_t* c, char* buffer, int buffer_size) {
    while (*c->current == ' ' || *c->current == '\t' || *c->current == '\n' || *c->current == '\r')
        c->current++;
    if (*c->current == '\0')
        return false;

    const char* e = min(strchr(c->current, '\r'), strchr(c->current, '\n'));
    e = (e) ? e : c->end;
    int n = min(min(e - c->current, c->len), buffer_size - 1);
    memcpy(buffer, c->current, n);
    buffer[n] = '\0';
    c->current += n;
    return true;
}

const char* skip_space(const char* s) {
    while (*s == ' ' || *s == '\t' || *s == '\n' || *s == '\r') s++;
    return s;
}


bool beatmap_load(const char* filepath, beatmap_t* beatmap, bool load_only_meta) {
    LOGF("Loading beatmap \"%s\" ...", filepath);

    clock_t load_start = clock();

    *beatmap = (beatmap_t){0};
    kv_init(beatmap->breaks);
    kv_init(beatmap->notes);
    kv_init(beatmap->timing_points);

    if (!FileExists(filepath)) {
        LOGF("File \"%s\" does not exists", filepath);
        return false;
    }
    STRCP(beatmap->beatmap_filepath, filepath);

    char* file_text = LoadFileText(filepath);
    if (file_text == NULL) {
        LOGF("Failed to read \"%s\"", filepath);
        return false;
    }

    int l = strlen(file_text);
    cursor_t cursor = {
        .start = file_text,
        .end = file_text + l,
        .len = l,
        .current = file_text
    };
    char line[100] = {'\0'};

    if (!cursor_get_next_line(&cursor, line, STACKARRAY_SIZE(line)) || TextFindIndex(line, "osu file format ") == -1) {
        LOGF("File \"%s\" is not an Osu beatmap", filepath);
        return false;
    }
    STRCP(beatmap->format_version, TextSubtext(line, 16, strlen(line) - 16));

    khint_t section = 0;
    khint_t section_general         = kh_str_hash_func("General");
    khint_t section_metadata        = kh_str_hash_func("Metadata");
    khint_t section_difficulty      = kh_str_hash_func("Difficulty");
    khint_t section_events          = kh_str_hash_func("Events");
    khint_t section_timing_points   = kh_str_hash_func("TimingPoints");
    khint_t section_hit_objects     = kh_str_hash_func("HitObjects");
    khint_t key_audio_filename      = kh_str_hash_func("AudioFilename");
    khint_t key_audio_lead_in       = kh_str_hash_func("AudioLeadIn");
    khint_t key_audio_preview_time  = kh_str_hash_func("PreviewTime");
    khint_t key_stack_leniency      = kh_str_hash_func("StackLeniency");
    khint_t key_mode                = kh_str_hash_func("Mode");
    khint_t key_title               = kh_str_hash_func("Title");
    khint_t key_artist              = kh_str_hash_func("Artist");
    khint_t key_creator             = kh_str_hash_func("Creator");
    khint_t key_difname             = kh_str_hash_func("Version");
    khint_t key_hp                  = kh_str_hash_func("HPDrainRate");
    khint_t key_cs                  = kh_str_hash_func("CircleSize");
    khint_t key_od                  = kh_str_hash_func("OverallDifficulty");
    khint_t key_ar                  = kh_str_hash_func("ApproachRate");
    khint_t key_sv                  = kh_str_hash_func("SliderMultiplier");
    khint_t key_str                 = kh_str_hash_func("SliderTickRate");
    while (cursor_get_next_line(&cursor, line, STACKARRAY_SIZE(line))) {
        int line_len = strlen(line);

        if (line[0] == '/' && line[1] == '/')
            continue;

        if (line[0] == '[' && line[line_len - 1] == ']') {
            const char* s = TextSubtext(line, 1, line_len - 2);
            section = kh_str_hash_func(s);
        }
        else {
            if (section == section_general || section == section_metadata || section == section_difficulty) {
                int delim_i = TextFindIndex(line, ":");
                if (delim_i == -1)
                    continue;

                char key_s[50] = {'\0'};
                memcpy(key_s, line, min(delim_i, STACKARRAY_SIZE(key_s)));
                char val_s[256] = {'\0'};
                memcpy(val_s, skip_space(line + delim_i + 1), min(strlen(skip_space(line + delim_i + 1)), STACKARRAY_SIZE(val_s)));
                int val_len = strlen(val_s);

                khint_t key = kh_str_hash_func(key_s);

                if (section == section_general) {
                    if (key == key_audio_filename) {
                        STRCP(beatmap->audio_filename, val_s);
                    }
                    else if (key == key_audio_lead_in) {
                        beatmap->audio_lead_in = strtof(val_s, NULL);
                    }
                    else if (key == key_audio_preview_time) {
                        beatmap->preview_time = strtof(val_s, NULL);
                    }
                    else if (key == key_stack_leniency) {
                        beatmap->stack_leniency = strtof(val_s, NULL);
                    }
                    else if (key == key_mode) {
                        beatmap->mode = strtof(val_s, NULL);
                        if (beatmap->mode != 3) {
                            LOG("Beatmap mode is not osu!mania");
                            return false;
                        }
                    }
                }
                else if (section == section_metadata) {
                    if (key == key_title) {
                        STRCP(beatmap->title, val_s);
                    }
                    else if (key == key_artist) {
                        STRCP(beatmap->artist, val_s);
                    }
                    else if (key == key_creator) {
                        STRCP(beatmap->creator, val_s);
                    }
                    else if (key == key_difname) {
                        STRCP(beatmap->difname, val_s);
                    }
                }
                else if (section == section_difficulty) {
                    if (key == key_hp) {
                        beatmap->HP = strtof(val_s, NULL);
                    }
                    else if (key == key_cs) {
                        beatmap->CS = strtof(val_s, NULL);
                    }
                    else if (key == key_od) {
                        beatmap->OD = strtof(val_s, NULL);
                    }
                    else if (key == key_ar) {
                        beatmap->AR = strtof(val_s, NULL);
                    }
                    else if (key == key_sv) {
                        beatmap->SV = strtof(val_s, NULL);
                    }
                    else if (key == key_str) {
                        beatmap->STR = strtof(val_s, NULL);
                    }
                }
            }
            else if (section == section_events || section == section_timing_points || section == section_hit_objects) {
                int params_count = 0;
                char** params = (char**)TextSplit(line, ',', &params_count);

                if (section == section_events) {
                    int event_type = params[0][0] - '0';

                    if (params_count < 3) {
                        LOGF("invalid event \"%s\"", line);
                        continue;
                    }

                    if (event_type == 0) {
                        params[2][strlen(params[2]) - 1] = '\0';
                        STRCP(beatmap->background_filename, params[2] + 1);
                    }
                    else if (event_type == 2) {
                        beatmap_break_t b = (beatmap_break_t) {
                            .time_start = atoi(params[1]),
                            .time_end = atoi(params[2])
                        };
                        kv_push(beatmap_break_t, beatmap->breaks, b);
                    }
                }
                else if (section == section_timing_points && !load_only_meta) {
                    if (params_count != 8) {
                        LOGF("invalid timing point \"%s\"", line);
                        continue;
                    }

                    beatmap_timing_point_t tm = {0};
                    tm.time_start = atoi(params[0]);
                    tm.length = atof(params[1]);
                    tm.meter = atoi(params[2]);
                    tm.is_uninherited = atoi(params[6]) == 1;

                    kv_push(beatmap_timing_point_t, beatmap->timing_points, tm);
                }
                else if (section == section_hit_objects && !load_only_meta) {
                    if (beatmap->CS == 0) {
                        LOG("Could not calculate hit object pararms beacause CS was not specified");
                        return false;
                    }

                    beatmap_note_t note = {0};

                    int type = atoi(params[3]);
                    if (type != 1 && type != 128)
                        continue;

                    note.is_hold_note = type == 128;
                    note.time_start = atoi(params[2]);
                    if (note.is_hold_note) {
                        int count = 0;
                        char* c = strchr(params[5], ':');
                        c[0] = '\0';
                        note.time_end = atoi(params[5]);
                    }

                    note.column = Clamp(
                        floorf(atoi(params[0]) * beatmap->CS / 512.0f),
                        0,
                        beatmap->CS - 1
                    );

                    kv_push(beatmap_note_t, beatmap->notes, note);
                }
            }
        }
    }

    UnloadFileText(file_text);

    bool ok = true;

    #define CHECK_REQUIRED_FIELD(field, config_name) \
        if (!(field)) {\
            LOG("Missing field \"" config_name "\"");\
            ok = false;\
        }
    CHECK_REQUIRED_FIELD(beatmap->audio_filename[0], "AudioFilename");
    CHECK_REQUIRED_FIELD(beatmap->background_filename[0], "<Background event>");
    CHECK_REQUIRED_FIELD(beatmap->title[0], "Title");
    CHECK_REQUIRED_FIELD(beatmap->artist[0], "Artist");
    CHECK_REQUIRED_FIELD(beatmap->creator[0], "Creator");
    CHECK_REQUIRED_FIELD(beatmap->difname[0], "Version");
    CHECK_REQUIRED_FIELD(beatmap->HP, "HPDrainRate");
    CHECK_REQUIRED_FIELD(beatmap->CS, "CircleSize");
    CHECK_REQUIRED_FIELD(beatmap->OD, "OverallDifficulty");
    CHECK_REQUIRED_FIELD(beatmap->AR, "ApproachRate");
    CHECK_REQUIRED_FIELD(beatmap->SV, "SliderMultiplier");
    CHECK_REQUIRED_FIELD(beatmap->STR, "SliderTickRate");

    if (!load_only_meta) {
        if (kv_size(beatmap->timing_points) == 0) {
            LOGF("No timing points in \"%s\"", filepath);
            ok = false;
        }
        if (kv_size(beatmap->notes) == 0) {
            LOGF("No notes in \"%s\"", filepath);
            ok = false;
        }
    }

    if (ok)
        LOGF("beatmap_load() took %f seconds", (float)(clock() - load_start) / CLOCKS_PER_SEC);

    return ok;
}

void beatmap_destroy(beatmap_t* beatmap) {
    kv_destroy(beatmap->breaks);
    kv_destroy(beatmap->timing_points);
    kv_destroy(beatmap->notes);
    kv_init(beatmap->breaks);
    kv_init(beatmap->timing_points);
    kv_init(beatmap->notes);
}

void beatmap_debug_print(beatmap_t* beatmap) {
    LOGF(
        "Beatmap:\n"
        "\tversion: %s\n"
        "\taudio: %s\n"
        "\tbackground: %s\n"
        "\taudio lead in: %f\n"
        "\tpreview time: %d\n"
        "\tstack leniency: %f\n"
        "\tmode: %d\n"
        "\ttitle: %s\n"
        "\tartist: %s\n"
        "\tcreator: %s\n"
        "\tdifname: %s\n"
        "\tHP: %f\n"
        "\tCS: %f\n"
        "\tOD: %f\n"
        "\tAR: %f\n"
        "\tSV: %f\n"
        "\tSTR: %f\n"
        "\tbreaks: %d\n"
        "\ttiming points: %d\n"
        "\thit objects: %d",
        beatmap->format_version,
        beatmap->audio_filename,
        beatmap->background_filename,
        beatmap->audio_lead_in,
        beatmap->preview_time,
        beatmap->stack_leniency,
        beatmap->mode,
        beatmap->title,
        beatmap->artist,
        beatmap->creator,
        beatmap->difname,
        beatmap->HP,
        beatmap->CS,
        beatmap->OD,
        beatmap->AR,
        beatmap->SV,
        beatmap->STR,
        kv_size(beatmap->breaks),
        kv_size(beatmap->timing_points),
        kv_size(beatmap->notes)
    );
}

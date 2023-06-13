/**
 * @defgroup   BEATMAP beatmap
 *
 * @brief      This file implements beatmap parser and loader.
 *             Reference: https://osu.ppy.sh/wiki/en/Client/File_formats/Osu_(file_format)
 *
 * @author     xfnty (github.com)
 * @date       2023
 */

#include <beatmap.h>

#include <stddef.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <math.h>

#define SCOPE_NAME "beatmap"
#include <logging.h>
#include <defines.h>

#include <humanize.h>
#include <raylib.h>
#include <raymath.h>
#include <zip.h>
#include <kvec.h>
#include <khash.h>
#include <ksort.h>
#include <ini.h>


/* constants */
// Calculated using kh_str_hash_func("Whatever")
// SECTION_NULL is triggered for "osu file format vXX"
#define SECTION_NULL           0
#define SECTION_GENERAL        1584505032
#define SECTION_METADATA       -385360049
#define SECTION_DIFFICULTY     -472001573
#define SECTION_EVENTS         2087505209
#define SECTION_TIMING_POINTS  -442323475
#define SECTION_HITOBJECTS     -1760687583
#define KEY_AUDIO_FILENAME     -1868215331
#define KEY_AUDIO_LEAD_IN      1895414007
#define KEY_PREVIEW_TIME       376647317
#define KEY_MODE               2403779
#define KEY_TITLE              80818744
#define KEY_TITLE_UNICODE      607220101
#define KEY_ARTIST             1969736551
#define KEY_ARTIST_UNICODE     -1526467210
#define KEY_CREATOR            -1601759220
#define KEY_VERSION            2016261304
#define KEY_SOURCES            -1812638661
#define KEY_TAGS               2598969
#define KEY_BEATMAP_ID         215463265
#define KEY_BEATMAPSET_ID      -2099647785
#define KEY_HP                 -1604895024
#define KEY_CS                 882574609
#define KEY_OD                 955053000
#define KEY_AR                 -1015867192
#define KEY_SV                 -215404126


/* macros */
#define STRCP(dest, src)\
    memcpy((void*)(dest), src, MIN(strlen(src), STACKARRAY_SIZE(dest) - 1));\
    (dest)[STACKARRAY_SIZE(dest) - 1] = '\0';


/* types */
typedef struct {
    char* data;
    size_t size;
    char name[STRSIZE];
} file_t;

typedef kvec_t(file_t) beatmapset_files_t;

typedef struct {
    beatmap_t*      beatmap;
    difficulty_t*   difficulty;
} ini_callback_args_t;


/* local functions */
static bool         load_files(beatmapset_files_t* files, const char* path);
static bool         parse_difficulty(file_t* file, beatmap_t* beatmap, difficulty_t* difficulty);
static bool         preprocess_difficulty_events(difficulty_t* difficulty);
static int          ini_callback(void* user, const char* section, const char* line, int lineno);
static const char*  skip_space(const char* s);
static bool         sort_hitobjects(hitobject_t a, hitobject_t b);
static bool         sort_timing_points(timing_point_t a, timing_point_t b);
KSORT_INIT(hitobject_t, hitobject_t, sort_hitobjects);
KSORT_INIT(timing_point_t, timing_point_t, sort_timing_points);


bool beatmap_load(beatmap_t* beatmap, const char* path) {
    assert(beatmap != NULL);
    assert(path != NULL);

    seconds_t load_time, parse_time, preprocess_time, calc_time;
    beatmapset_files_t files;

    memset(beatmap, 0, sizeof(beatmap_t));

    LOGF("loading beatmap \"%s\" ...", path);
    if (!load_files(&files, path))
        return false;

    LOG("parsing beatmap ...");
    for (int i = 0; i < kv_size(files); i++) {
        difficulty_t diff;
        if (parse_difficulty(&kv_A(files, i), beatmap, &diff)) {
            kv_push(difficulty_t, beatmap->difficulties, diff);
        }
    }

    for (int i = 0; i < kv_size(files); i++) {
        free(kv_A(files, i).data);
    }

    return true;
}

void beatmap_destroy(beatmap_t* beatmap) {
    assert(beatmap != NULL);

    if (kv_size(beatmap->difficulties)) {
        for (int i = 0; i < kv_size(beatmap->difficulties); i++) {
            difficulty_t* d = &kv_A(beatmap->difficulties, i);

            if (kv_size(d->timing_points))  kv_destroy(d->timing_points);
            if (kv_size(d->hitobjects))     kv_destroy(d->hitobjects);
        }

        kv_destroy(beatmap->difficulties);
    }
}

void beatmap_debug_print(beatmap_t* beatmap) {
    assert(beatmap != NULL);

    LOG("\nBeatmap info:");
    printf(
        "\tid: %d\n"
        "\ttitle: %s\n",
        beatmap->id,
        beatmap->title
    );
    for (int i = 0; i < kv_size(beatmap->difficulties); i++) {
        difficulty_t* d = &kv_A(beatmap->difficulties, i);
        printf(
            "Difficulty[%d]\n"
            "\tid: %d\n"
            "\tname: %s\n"
            "\taudio: %s\n"
            "\tCS: %.1f\n"
            "\tSV: %.1f\n",
            i,
            d->id,
            d->name,
            d->audio_filename,
            d->CS,
            d->SV
        );

        const int MAX_OBJECTS_SHOWN = 100;

        printf("\tTiming points[%lu]:\n", kv_size(d->timing_points));
        for (int j = 0; j < MIN(MAX_OBJECTS_SHOWN, kv_size(d->timing_points)); j++) {
            timing_point_t* tm = &kv_A(d->timing_points, j);
            printf("\t\tTM[%d] at %f SV=%f BPM=%f Y=%f\n", j, tm->time, tm->SV, tm->BPM, tm->y);
        }
        if (kv_size(d->timing_points) > MAX_OBJECTS_SHOWN)
            printf("\t\t...\n");

        printf("\tHit objects[%lu]:\n", kv_size(d->hitobjects));
        for (int j = 0; j < MIN(MAX_OBJECTS_SHOWN, kv_size(d->hitobjects)); j++) {
            hitobject_t* ho = &kv_A(d->hitobjects, j);
            printf("\t\tHO[%d] at %f from %f COL=%d YS=%f YE=%f\n", j, ho->start_time, ho->end_time, ho->column, ho->start_y, ho->end_y);
        }
        if (kv_size(d->hitobjects) > MAX_OBJECTS_SHOWN)
            printf("\t\t...\n");
    }
}

int difficulty_get_timing_point_index_for_time(difficulty_t* difficulty, seconds_t time) {
    assert(difficulty != NULL);

    if (kv_size(difficulty->timing_points) == 0)
        return -1;

    int i = 0;
    for (; i < kv_size(difficulty->timing_points); i++) {
        timing_point_t* tm = &kv_A(difficulty->timing_points, i);

        if (tm->time > time)
            return i - 1;
    }

    return i;
}

bool load_files(beatmapset_files_t* files, const char* path) {
    assert(files != NULL);
    assert(path != NULL);

    kv_init(*files);

    if (TextIsEqual(GetFileExtension(path), ".osz")) {
        struct zip_t* zip = zip_open(path, 0, 'r');
        if (zip == NULL) {
            LOGF("Failed to open \"%s\" as a zip file", path);
            return false;
        }

        int n = zip_entries_total(zip);
        for (int i = 0; i < n; i++) {
            if (zip_entry_openbyindex(zip, i) != 0) {
                LOGF("Failed to open zip entry %d in \"%s\"", i, path);
                return false;
            }

            const char *name = zip_entry_name(zip);
            if (name == NULL) {
                LOGF("Recieved NULL name on entry %d in \"%s\"", i, path);
                return false;
            }

            if (!zip_entry_isdir(zip) && TextIsEqual(GetFileExtension(name), ".osu")) {
                file_t f;
                f.size = zip_entry_size(zip) + 1;
                f.data = malloc(f.size);
                strncpy(f.name, name, STACKARRAY_SIZE(f.name));

                ssize_t br = zip_entry_noallocread(zip, f.data, f.size);
                if (br <= 0) {
                    LOGF("Failed to read zip entry: %d", br);
                    return false;
                }
                f.data[br] = '\0';

                kv_push(file_t, *files, f);

                LOGF("loaded \"%s\" (%s)", name, humanize_bytesize(f.size));
            }
            zip_entry_close(zip);
        }
    }
    else if (DirectoryExists(path)) {
        FilePathList fs = LoadDirectoryFilesEx(path, ".osu", false);
        for (int i = 0; i < fs.count; i++) {
            file_t f;

            FILE *file = fopen(fs.paths[i], "rb");

            fseek(file, 0, SEEK_END);
            f.size = ftell(file) + 1;
            fseek(file, 0, SEEK_SET);

            f.data = malloc(f.size);
            size_t count = fread(f.data, 1, f.size, file);
            f.data[count] = '\0';

            strncpy(f.name, GetFileName(fs.paths[i]), STACKARRAY_SIZE(f.name));

            if (f.data == NULL || f.size == 0) {
                LOGF("Could not read \"%s\"", fs.paths[i]);
                return false;
            }

            kv_push(file_t, *files, f);

            LOGF("loaded \"%s\" (%s)", GetFileName(fs.paths[i]), humanize_bytesize(f.size));
        }
    }
    else {
        LOGF("\"%s\" is not a valid osu file or a beatmap directory", path);
        return false;
    }

    size_t total_size = 0;
    for (int i = 0; i < kv_size(*files); i++)
        total_size += kv_A(*files, i).size;
    LOGF("total beatmap size is %s", humanize_bytesize(total_size));

    return true;
}

bool parse_difficulty(file_t* file, beatmap_t* beatmap, difficulty_t* difficulty) {
    assert(file != NULL);
    assert(beatmap != NULL);
    assert(difficulty != NULL);

    memset(difficulty, 0, sizeof(difficulty_t));
    STRCP(difficulty->file_name, file->name);

    ini_callback_args_t args = { beatmap, difficulty };
    int err = ini_parse_string(file->data, ini_callback, &args);
    if (err > 0) {
        return false;
    }
    else if (err < 0) {
        LOGF("failed to parse \"%s\": file IO error", difficulty->file_name);
        return false;
    }

    ks_introsort(hitobject_t, kv_size(difficulty->hitobjects), difficulty->hitobjects.a);
    ks_introsort(timing_point_t, kv_size(difficulty->timing_points), difficulty->timing_points.a);

    LOGF("parsed \"%s\"", difficulty->file_name);
    return true;
}

int ini_callback(void* user, const char* section, const char* line, int lineno) {
    assert(user != NULL);
    assert(section != NULL);
    assert(line != NULL);

    ini_callback_args_t* args = (ini_callback_args_t*)user;

    char key[64]        = {0};
    char value[256]     = {0};
    char** params       = NULL;
    int params_count    = 0;
    int event_type      = 0;

    khint_t key_hash;
    khint_t section_hash = (section) ? kh_str_hash_func(section) : 0;

    if (section_hash == SECTION_GENERAL || section_hash == SECTION_METADATA || section_hash == SECTION_DIFFICULTY) {
        int delim_i = TextFindIndex(line, ":");
        memcpy(key, line, MIN(delim_i, STACKARRAY_SIZE(key)));
        memcpy(value, skip_space(line + delim_i + 1), MIN(strlen(skip_space(line + delim_i + 1)), STACKARRAY_SIZE(value)));
        key_hash = kh_str_hash_func(key);
    }
    else {
        params_count = 0;
        params = (char**)TextSplit(line, ',', &params_count);
        event_type = params[0][0] - '0';
    }

    switch (section_hash) {
    case SECTION_GENERAL:
        switch (key_hash) {
        case KEY_AUDIO_FILENAME:
            STRCP(args->difficulty->audio_filename, value);
            break;

        case KEY_MODE:
            if (atoi(value) != 3) {
                LOGF("failed to parse \"%s\": not an osu!mania beatmap (%s)", args->difficulty->file_name, line);
                return false;
            }
            break;
        }
        break;

    case SECTION_METADATA:
        switch (key_hash) {
        case KEY_TITLE:
            if (args->beatmap->title[0] == '\0')
                STRCP(args->beatmap->title, value);
            break;

        case KEY_VERSION:
            STRCP(args->difficulty->name, value);
            break;

        case KEY_BEATMAP_ID:
            args->difficulty->id = atoi(value);
            break;

        case KEY_BEATMAPSET_ID:
            args->beatmap->id = atoi(value);
            break;
        }
        break;

    case SECTION_DIFFICULTY:
        switch (key_hash) {
        case KEY_CS:
            args->difficulty->CS = atof(value);
            break;

        case KEY_SV:
            args->difficulty->SV = atof(value);
            break;
        }
        break;

    case SECTION_TIMING_POINTS:
        if (params_count != 8) {
            LOGF("failed to parse \"%s\": invalid timing point (\"%s\" at line %d)", args->difficulty->file_name, line, lineno);
            return false;
        }

        seconds_t       start_time      = atoi(params[0]) / 1000.0f;
        seconds_t       beat_length     = atof(params[1]);
        int             meter           = atoi(params[2]);
        int             sample_set      = atoi(params[3]);
        int             sample_index    = atoi(params[4]);
        percentage_t    volume          = atoi(params[5]) / 100.0f;
        bool            is_uninherited  = atoi(params[6]) == 1;
        bool            effects         = atoi(params[7]);
        bool            is_first_tm     = kv_size(args->difficulty->timing_points) == 0;

        if (is_first_tm && !is_uninherited) {
            LOGF("failed to parse \"%s\": first timing point can not be inhereited (\"%s\" at line %d)", args->difficulty->file_name, line, lineno);
            return false;
        }

        timing_point_t prev_tm = (!is_first_tm)
            ? (kv_A(args->difficulty->timing_points, kv_size(args->difficulty->timing_points) - 1))
            : ((timing_point_t){0});

        float BPM = (is_uninherited) ? (roundf(60000 / beat_length)) : (prev_tm.BPM);
        float SV = (is_uninherited) ? (args->difficulty->SV) : (args->difficulty->SV * (100.0f / (float)(-beat_length)));

        timing_point_t tm = (timing_point_t) {
            .time           = start_time,
            .BPM            = BPM,
            .SV             = SV,
            .y              = (is_first_tm) ? (0) : (prev_tm.y + (100 * prev_tm.SV) * (start_time - prev_tm.time) / (60.0f / prev_tm.BPM))
        };  // FIXME:                           \_  this is probably untrue because tm.time of the first timing point might be negative.
            //                                      It means that it should probably be calculated with parameters of the current timing point.
        kv_push(timing_point_t, args->difficulty->timing_points, tm);
        break;

    case SECTION_HITOBJECTS:
        if (params_count < 5) {
            LOGF("failed to parse \"%s\": invalid hit object (\"%s\" at line %d)", args->difficulty->file_name, line, lineno);
            return false;
        }

        int         x           = atoi(params[0]);
        int         y           = atoi(params[1]);
        seconds_t   time_strt   = atoi(params[2]) / 1000.0f; // FIXME: WTFFF
        int         type        = atoi(params[3]);
        int         hitsound    = atoi(params[4]);
        bool        is_hold     = type == 128;
        int         column      = Clamp(
            floorf(atoi(params[0]) * args->difficulty->CS / 512.0f),
            0,
            args->difficulty->CS - 1
        );

        if (is_hold)
            strchr(params[5], ':')[0] = '\0';

        int i = difficulty_get_timing_point_index_for_time(args->difficulty, time_strt);
        if (i < 0) {
            LOGF(
                "failed to parse \"%s\":"
                " hit object %d does not have associated timing point"
                " (\"%s\" at line %d)",
                args->difficulty->file_name,
                kv_size(args->difficulty->hitobjects) - 1,
                line,
                lineno
            );
            return false;
        }
        timing_point_t* atm = &kv_A(args->difficulty->timing_points, i);

        hitobject_t ho = (hitobject_t) {
            .column     = column,
            .start_time = time_strt,
            .start_y    = atm->y + (100 * atm->SV) * (time_strt - atm->time) / (60.0f / atm->BPM),
            .end_time   = 0,// (is_hold) ? atoi(params[5]) / 1000.0f : 0,
            .end_y      = 0
        };

        kv_push(hitobject_t, args->difficulty->hitobjects, ho);
        break;

    case SECTION_NULL:
        break;
    }

    return true;
}

const char* skip_space(const char* s) {
    assert(s != NULL);

    while (*s == ' ' || *s == '\t' || *s == '\n' || *s == '\r') s++;
    return s;
}

bool sort_hitobjects(hitobject_t a, hitobject_t b) {
    return a.start_time < b.start_time;
}

bool sort_timing_points(timing_point_t a, timing_point_t b) {
    return a.time < b.time;
}

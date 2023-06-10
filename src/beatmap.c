#include <beatmap.h>

#include <stddef.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

#define SCOPE_NAME "beatmap"
#include <logging.h>
#include <defines.h>

#include <humanize.h>
#include <raylib.h>
#include <zip.h>
#include <kvec.h>
#include <khash.h>
#include <ini.h>


/* constants */
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
#define KEY_ARTIST             1969736551
#define KEY_CREATOR            -1601759220
#define KEY_VERSION            2016261304
#define KEY_HP                 -1604895024
#define KEY_CS                 882574609
#define KEY_OD                 955053000
#define KEY_AR                 -1015867192
#define KEY_SV                 -215404126


/* macros */
#define STRCP(dest, src)\
    memcpy((void*)(dest), src, MIN(strlen(src), STACKARRAY_SIZE(dest) - 1));\
    (dest)[STACKARRAY_SIZE(dest) - 1] = '\0';

#define TIME() (clock() / (float)CLOCKS_PER_SEC)


/* types */
typedef struct {
    char* data;
    size_t size;
    char name[STRSIZE];
} file_t;

typedef kvec_t(file_t) beatmapset_files_t;

typedef struct {
    difficulty_t*   difficulty;
    file_t*         file;
} ini_callback_args_t;


/* local functions */
static bool load_files(beatmapset_files_t* files, const char* path);
static bool parse_difficulty(file_t file, difficulty_t* difficulty);
static int  ini_callback(void* user, const char* section, const char* line, int lineno);
static const char* skip_space(const char* s);


bool beatmap_load(beatmap_t* beatmap, const char* path) {
    assert(beatmap != NULL);
    assert(path != NULL);

    seconds_t load_time, parse_time, preprocess_time, calc_time;
    beatmapset_files_t files;

    memset(beatmap, 0, sizeof(beatmap_t));

    LOGF("loading beatmap \"%s\" ...", path);
    load_time = TIME();
    if (!load_files(&files, path))
        return false;
    load_time = TIME() - load_time;

    LOG("parsing beatmap ...");
    parse_time = TIME();
    for (int i = 0; i < kv_size(files); i++) {
        difficulty_t diff;
        if (parse_difficulty(kv_A(files, i), &diff))
            kv_push(difficulty_t, beatmap->difficulties, diff);
    }
    parse_time = TIME() - parse_time;

    preprocess_time = TIME();
    preprocess_time = TIME() - preprocess_time;

    calc_time = TIME();
    calc_time = TIME() - calc_time;

    LOGF(
        "beatmap_load():\n"
        "\tload: %fs\n"
        "\tparse: %fs\n"
        "\tpreprocess: %fs\n"
        "\tcalc: %fs\n"
        "\tTOTAL: %fs",
        load_time,
        parse_time,
        preprocess_time,
        calc_time,
        load_time + parse_time + preprocess_time + calc_time
    );

    for (int i = 0; i < kv_size(files); i++) {
        free(kv_A(files, i).data);
    }

    return true;
}

void beatmap_destroy(beatmap_t* beatmap) {
}

void beatmap_debug_print(beatmap_t* beatmap) {
}

timing_point_t* DIFFICULTY_get_timing_point_for(difficulty_t* difficulty, seconds_t time) {
    return NULL;
}

playfield_event_t* DIFFICULTY_get_playfield_event_for(difficulty_t* difficulty, seconds_t time, int column, bool find_nearest) {
    return NULL;
}

bool load_files(beatmapset_files_t* files, const char* path) {
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

bool parse_difficulty(file_t file, difficulty_t* difficulty) {
    memset(difficulty, 0, sizeof(difficulty_t));

    ini_callback_args_t args = { difficulty, &file };
    int err = ini_parse_string(file.data, ini_callback, &args);
    if (err > 0) {
        return false;
    }
    else if (err < 0) {
        LOGF("failed to parse \"%s\": file IO error", file.name);
        return false;
    }

    LOGF("parsed \"%s\"", file.name);
    return true;
}

int ini_callback(void* user, const char* section, const char* line, int lineno) {
    ini_callback_args_t* args = (ini_callback_args_t*)user;

    char key[64]        = {0};
    char value[256]     = {0};
    char** params       = NULL;
    int params_count    = 0;

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
    }

    switch (section_hash) {
    case SECTION_GENERAL:
        switch (key_hash) {
        case KEY_AUDIO_FILENAME:
            STRCP(args->difficulty->audio_filename, value);
            break;
        case KEY_AUDIO_LEAD_IN:
            args->difficulty->audio_lead_in = atof(value);
            break;
        case KEY_PREVIEW_TIME:
            args->difficulty->preview_time = atof(value);
            break;
        case KEY_MODE:
            if (atoi(value) != 3) {
                LOGF("failed to parse \"%s\": not an osu!mania beatmap (%s)", args->file->name, line);
                return false;
            }
            break;
        }
        break;

    case SECTION_METADATA:
        switch (key_hash) {
        case KEY_TITLE:
            break;
        case KEY_ARTIST:
            break;
        case KEY_CREATOR:
            break;
        case KEY_VERSION:
            STRCP(args->difficulty->name, value);
            break;
        }
        break;

    case SECTION_DIFFICULTY:
        switch (key_hash) {
        case KEY_HP:
            args->difficulty->HP = atof(value);
            break;
        case KEY_OD:
            args->difficulty->OD = atof(value);
            break;
        case KEY_CS:
            args->difficulty->CS = atof(value);
            break;
        case KEY_AR:
            args->difficulty->AR = atof(value);
            break;
        case KEY_SV:
            args->difficulty->SV = atof(value);
            break;
        }
        break;

    case SECTION_EVENTS:
        break;

    case SECTION_TIMING_POINTS:
        break;

    case SECTION_HITOBJECTS:
        break;

    case SECTION_NULL:
        // printf("format version is v%d\n", atoi(TextSubtext(line, 17, strlen(line) - 17)));
        break;
    }

    return true;
}

const char* skip_space(const char* s) {
    while (*s == ' ' || *s == '\t' || *s == '\n' || *s == '\r') s++;
    return s;
}

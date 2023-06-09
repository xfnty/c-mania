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


/* macros */
#define STRCP(dest, src)\
    memcpy((void*)(dest), src, MIN(strlen(src), STACKARRAY_SIZE(dest) - 1));\
    (dest)[STACKARRAY_SIZE(dest) - 1] = '\0';


/* types */
typedef struct {
    unsigned char* data;
    size_t size;
    char name[STRSIZE];
} file_t;

typedef kvec_t(file_t) beatmapset_files_t;

typedef struct {
    struct zip_t* zip;
    beatmapset_files_t* files;
    const char* path;
    int i;
} zip_callback_arg_t;

typedef struct cursor_s {
    const char* start;
    const char* end;
    int len;
    const char* current;
} cursor_t;


/* local functions */
static bool load_files(beatmapset_files_t* files, const char* path);
static bool parse_difficulty(file_t file, difficulty_t* difficulty);
static bool cursor_get_next_line(cursor_t* c, char* buffer, int buffer_size);
static const char* skip_space(const char* s);


bool beatmap_load(beatmap_t* beatmap, const char* path) {
    assert(beatmap != NULL);
    assert(path != NULL);

    clock_t stage_start_time;
    beatmapset_files_t files;

    memset(beatmap, 0, sizeof(beatmap_t));

    // 1. Load .osu files into memory
    LOGF("loading beatmap \"%s\" ...", path);
    stage_start_time = clock();
    if (!load_files(&files, path))
        return false;
    seconds_t load_duration = (clock() - stage_start_time) / (float)CLOCKS_PER_SEC;

    // 2. Parse .osu buffers
    LOG("parsing beatmap ...");
    stage_start_time = clock();
    for (int i = 0; i < kv_size(files); i++) {
        difficulty_t diff;
        if (!parse_difficulty(kv_A(files, i), &diff))
            return false;
        kv_push(difficulty_t, beatmap->difficulties, diff);
    }
    seconds_t parse_duration = (clock() - stage_start_time) / (float)CLOCKS_PER_SEC;

    // 3. Preprocess playfield events
    stage_start_time = clock();
    seconds_t preprocess_duration = (clock() - stage_start_time) / (float)CLOCKS_PER_SEC;

    // 4. Calculate star rating
    stage_start_time = clock();
    seconds_t calc_duration = (clock() - stage_start_time) / (float)CLOCKS_PER_SEC;

    LOGF(
        "beatmap_load():\n"
        "\tload: %f\n"
        "\tparse: %f\n"
        "\tpreprocess: %f\n"
        "\tcalc: %f\n"
        "\tTOTAL: %f",
        load_duration,
        parse_duration,
        preprocess_duration,
        calc_duration,
        load_duration + parse_duration + preprocess_duration + calc_duration
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

timing_point_t* difficulty_get_timing_point_for(difficulty_t* difficulty, seconds_t time) {
    return NULL;
}

playfield_event_t* difficulty_get_playfield_event_for(difficulty_t* difficulty, seconds_t time, int column, bool find_nearest) {
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

    return true;
}

/*
static const int hashes[] = {
        1584505032,     // "General"
        -385360049,     // "Metadata"
        -472001573,     // "Difficulty"
        2087505209,     // "Events"
        -442323475,     // "TiMINgPoints"
        -1760687583,    // "HitObjects"
        -1868215331,    // "AudioFilename"
        1895414007,     // "AudioLeadIn"
        376647317,      // "PreviewTime"
        603842651,      // "StackLeniency"
        2403779,        // "Mode"
        80818744,       // "Title"
        1969736551,     // "Artist"
        -1601759220,    // "Creator"
        2016261304,     // "Version"
        -1604895024,    // "HPDrainRate"
        882574609,      // "CircleSize"
        955053000,      // "OverallDifficulty"
        -1015867192,    // "ApproachRate"
        -215404126,     // "SliderMultiplier"
        169882686,      // "SliderTickRate"
    };
*/
bool parse_difficulty(file_t file, difficulty_t* difficulty) {
    memset(difficulty, 0, sizeof(*difficulty));

    char* text = (char*)file.data;

    int l = strlen(text);
    cursor_t cursor = {
        .start = text,
        .end = text + l,
        .len = l,
        .current = text
    };
    char line[100] = {'\0'};

    if (!cursor_get_next_line(&cursor, line, STACKARRAY_SIZE(line)) || TextFindIndex(line, "osu file format ") == -1) {
        LOGF("File \"%s\" is not an Osu beatmap", file.name);
        return false;
    }

    int format_version = atoi(TextSubtext(line, 17, strlen(line) - 17));

    LOGF("parsed \"%s\" (v%d)", file.name, format_version);

    return true;
}

bool cursor_get_next_line(cursor_t* c, char* buffer, int buffer_size) {
    while (*c->current == ' ' || *c->current == '\t' || *c->current == '\n' || *c->current == '\r')
        c->current++;
    if (*c->current == '\0')
        return false;

    const char* e = MIN(strchr(c->current, '\r'), strchr(c->current, '\n'));
    e = (e) ? e : c->end;
    int n = MIN(MIN(e - c->current, c->len), buffer_size - 1);
    memcpy(buffer, c->current, n);
    buffer[n] = '\0';
    c->current += n;
    return true;
}

const char* skip_space(const char* s) {
    while (*s == ' ' || *s == '\t' || *s == '\n' || *s == '\r') s++;
    return s;
}

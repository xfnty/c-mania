#include "defines.h"
#include "humanize.h"
#include <beatmap.h>

#include <stddef.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

#define SCOPE_NAME "beatmap"
#include <logging.h>

#include <raylib.h>
#include <zip.h>
#include <kvec.h>


typedef struct {
    unsigned char* data;
    size_t size;
} glob_t;

typedef kvec_t(glob_t) beatmapset_files_t;

typedef struct {
    struct zip_t* zip;
    beatmapset_files_t* files;
    const char* path;
    int i;
} zip_callback_arg_t;

static bool load_files(beatmapset_files_t* files, const char* path);
static bool load_osz_files(beatmapset_files_t* files, const char* path);
static bool load_directory_files(beatmapset_files_t* files, const char* path);
static void unload_files(beatmapset_files_t* files);

static bool parse_difficulty(glob_t file, difficulty_t* difficulty);

static unsigned long on_extract(void *arg, unsigned long offset, const void *data, size_t size);

bool beatmap_load(beatmap_t* beatmap, const char* path) {
    // 1. Load .osu files into char buffers
    // 2. Parse buffers (load metadata, resources, hitobjects, timing points and breaks)
    // 3. Preprocess playfield events
    // 4. Calculate star rating

    assert(beatmap != NULL);
    assert(path != NULL);

    LOGF("Loading beatmap \"%s\" ...", path);
    clock_t load_start_time = clock();

    beatmapset_files_t files;
    kv_init(files);

    if (!load_files(&files, path))
        return false;

    for (int i = 0; i < kv_size(files); i++) {
        difficulty_t diff;
        if (!parse_difficulty(kv_A(files, i), &diff))
            return false;
    }

    unload_files(&files);

    LOGF("beatmap_load() took %f seconds", (float)(clock() - load_start_time) / CLOCKS_PER_SEC);
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
    LOGF("Loading \"%s\" ...", path);
    if (TextIsEqual(GetFileExtension(path), ".osz")) {
        if (!load_osz_files(files, path))
            return false;
    }
    else {
        if (!load_directory_files(files, path))
            return false;
    }

    return true;
}

void unload_files(beatmapset_files_t* files) {
    for (int i = 0; i < kv_size(*files); i++) {
        free(kv_A(*files, i).data);
    }
}

bool load_osz_files(beatmapset_files_t* files, const char* path) {
    if (!FileExists(path)) {
        LOGF("file \"%s\" does not exists", path);
        return false;
    }

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
            zip_callback_arg_t arg = (zip_callback_arg_t) { zip, files, path, i };
            zip_entry_extract(zip, on_extract, &arg);
        }

        zip_entry_close(zip);
    }

    return true;
}

bool load_directory_files(beatmapset_files_t* files, const char* path) {
    if (!DirectoryExists(path)) {
        LOGF("directory \"%s\" does not exists", path);
        return false;
    }

    FilePathList fs = LoadDirectoryFilesEx(path, ".osu", false);
    for (int i = 0; i < fs.count; i++) {
        glob_t g;

        FILE *file = fopen(fs.paths[i], "rb");

        fseek(file, 0, SEEK_END);
        g.size = ftell(file);
        fseek(file, 0, SEEK_SET);

        g.data = malloc(g.size);
        size_t count = fread(g.data, 1, g.size, file);

        if (g.data == NULL || g.size == 0) {
            LOGF("Could not read \"%s\"", fs.paths[i]);
            return false;
        }

        kv_push(glob_t, *files, g);

        char ss[45];
        humanize_bytesize(g.size, ss, STACKARRAY_SIZE(ss));
        LOGF("Loaded \"%s\" (%s)", GetFileName(fs.paths[i]), ss);
    }

    return true;
}

bool parse_difficulty(glob_t file, difficulty_t* difficulty) {
    static const int hashes[] = {
        1584505032,     // "General"
        -385360049,     // "Metadata"
        -472001573,     // "Difficulty"
        2087505209,     // "Events"
        -442323475,     // "TimingPoints"
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


    return true;
}

unsigned long on_extract(void *arg, unsigned long offset, const void *data, size_t size) {
    zip_callback_arg_t* args = (zip_callback_arg_t*)arg;

    struct zip_t* zip = args->zip;
    beatmapset_files_t* files = args->files;
    const char* path = args->path;
    const char* name = zip_entry_name(zip);
    int i = args->i;

    glob_t g;
    g.size = size;
    g.data = malloc(g.size);
    memcpy(g.data, data, g.size);
    kv_push(glob_t, *files, g);

    char ss[45];
    humanize_bytesize(g.size, ss, STACKARRAY_SIZE(ss));
    LOGF("Loaded \"%s\" (%s)", name, ss);

    return g.size;
}

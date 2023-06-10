#include <beatmap.h>

#include <stddef.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

#include <defines.h>

#include <humanize.h>
#include <raylib.h>
#include <zip.h>
#include <kvec.h>
#include <logging.h>


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

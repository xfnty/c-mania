#include <raylib.h>
#include <raymath.h>

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <sys/types.h>

#include <kvec.h>
#include <khash.h>
#include <ini.h>

#include <beatmap.h>
#include <logging.h>
#include <defines.h>

#include "./calc_hashes.h"
#include "./load_osz.h"
#include "humanize.h"


int ini_callback(void* user, const char* section, const char* line, int lineno) {
    printf("%4.d | \"%s\" (%s)\n", lineno, line, section);
    return true;
}

int main(int argc, char const *argv[])
{
    logging_init();

    beatmapset_files_t files;
    kv_init(files);

    load_files(&files, "assets/test.osz");

    for (int i = 0; i < kv_size(files) && i <= 1; i++) {
        file_t* file = &kv_A(files, i);
        printf("File \"%s\" (%s)\n", file->name, humanize_bytesize(file->size));
        ini_parse_string(file->data, ini_callback, NULL);
    }

    logging_shutdown();
    return 0;
}

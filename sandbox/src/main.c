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

#include <logging.h>
#include <defines.h>

#include "./calc_hashes.h"
#include "./load_osz.h"


const char* data =
"osu file format v14\n"
"[General]\n"
"AudioFilename: Flower Dance.mp3\n"
"AudioLeadIn: 0\n"
"PreviewTime: 103247\n"
"Countdown: 0\n"
"SampleSet: Soft\n"
"StackLeniency: 0.7\n"
"Mode: 0\n"
"LetterboxInBreaks: 0\n"
"WidescreenStoryboard: 0\n";

int main(int argc, char const *argv[])
{
    logging_init();

    beatmapset_files_t files;
    kv_init(files);

    // LOGF("load dir: %d", load_directory_files(&files, "assets/test"));
    load_osz_files(&files, "assets/test.osz");

    for (int i = 0; i < kv_size(files) && i <= 1; i++)
        printf("%s", kv_A(files, i).data);

    logging_shutdown();
    return 0;
}

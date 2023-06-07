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


#define f(s) printf("%d, // \"" s "\"\n", kh_str_hash_func(s));

int main(int argc, char const *argv[])
{
    logging_init();

    f("General");
    f("Metadata");
    f("Difficulty");
    f("Events");
    f("TimingPoints");
    f("HitObjects");
    f("AudioFilename");
    f("AudioLeadIn");
    f("PreviewTime");
    f("StackLeniency");
    f("Mode");
    f("Title");
    f("Artist");
    f("Creator");
    f("Version");
    f("HPDrainRate");
    f("CircleSize");
    f("OverallDifficulty");
    f("ApproachRate");
    f("SliderMultiplier");
    f("SliderTickRate");

    logging_shutdown();
    return 0;
}

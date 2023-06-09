#include <stdio.h>
#include <khash.h>

#define f(s) printf("%d, // \"" s "\"\n", kh_str_hash_func(s));


void calc() {
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
}

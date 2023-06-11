#include <stdio.h>
#include <khash.h>

#define f(s) printf("%d, // \"" s "\"\n", kh_str_hash_func(s));


void calc() {
    f("TitleUnicode");
    f("ArtistUnicode");
    f("Source");
    f("Tags");
    f("BeatmapID");
    f("BeatmapSetID");
}

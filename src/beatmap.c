#include <beatmap.h>

#include <zip.h>


bool beatmap_load(beatmap_t* beatmap, const char* path) {
    // 1. Load .osu files into char buffers
    // 2. Parse buffers (load metadata, resources, hitobjects, timing points and breaks)
    // 3. Preprocess playfield events
    // 4. Calculate star rating
    return false;
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

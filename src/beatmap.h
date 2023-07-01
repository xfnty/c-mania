/* References:
 *     https://osu.ppy.sh/wiki/en/Beatmap
 *     https://osu.ppy.sh/wiki/en/Client/File_formats/Osu_(file_format)
 */
#ifndef BEATMAP_H
#define BEATMAP_H

#include <stdbool.h>

#include <kvec.h>
#include <raylib.h>

#include "util.h"


/* types */
typedef float seconds_t;
typedef float percentage_t;  // 1.0 is 100%

typedef struct {
    seconds_t               time;
    float                   BPM;
    float                   SV;
    float                   y;
} timing_point_t;

typedef struct {
    seconds_t   start_time;
    float       start_y;
    seconds_t   end_time;  // nonzero for hold note
    float       end_y;
    int         column;
} hitobject_t;

typedef struct {
    id_t id;
    char name[256];
    char file_name[256];
    char audio_filename[256];

    float CS;  // column count in osu!mania
    float SV;

    kvec_t(timing_point_t)  timing_points;
    kvec_t(hitobject_t)     hitobjects;
} difficulty_t;

typedef struct {
    id_t id;
    char title[256];

    kvec_t(difficulty_t) difficulties;
} beatmap_t;


/* function declarations */
error_t beatmap_load(beatmap_t* beatmap, const char* path);
void    beatmap_destroy(beatmap_t* beatmap);

timing_point_t* difficulty_get_timing_point_for_time(difficulty_t* difficulty, seconds_t time);
int difficulty_get_timing_point_index_for_time(difficulty_t* difficulty, seconds_t time);


#endif

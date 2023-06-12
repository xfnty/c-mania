/* References:
 *     https://osu.ppy.sh/wiki/en/Beatmap
 *     https://osu.ppy.sh/wiki/en/Client/File_formats/Osu_(file_format)
 */
#ifndef BEATMAP_H
#define BEATMAP_H

#include <stdbool.h>

#include <kvec.h>
#include <raylib.h>

#include <defines.h>


/* macros */
#ifndef STRSIZE
#define STRSIZE 128
#endif


/* types */
typedef float seconds_t;
typedef float percentage_t;  // 1.0 is 100%

typedef struct {
    seconds_t               time;
    float                   BPM;
    float                   SV;
} timing_point_t;

typedef struct {
    seconds_t   start_time;
    seconds_t   end_time;  // nonzero for hold note
    int         column;
} hitobject_t;

typedef struct {
    id_t id;
    char name[STRSIZE];
    char file_name[STRSIZE];
    char audio_filename[STRSIZE];

    float CS;  // column count in osu!mania
    float SV;

    kvec_t(timing_point_t)  timing_points;
    kvec_t(hitobject_t)     hitobjects;
} difficulty_t;

typedef struct {
    id_t id;
    char title[STRSIZE];

    kvec_t(difficulty_t) difficulties;
} beatmap_t;


/* function declarations */
bool beatmap_load(beatmap_t* beatmap, const char* path);
void beatmap_destroy(beatmap_t* beatmap);
void beatmap_debug_print(beatmap_t* beatmap);


#endif

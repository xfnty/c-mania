#ifndef BEATMAP_H
#define BEATMAP_H

#include <stdbool.h>
#include <defines.h>
#include <kvec.h>
#include <raylib.h>

#ifndef STRSIZE
#define STRSIZE 128
#endif


/*  References:
 *      https://osu.ppy.sh/wiki/en/Beatmap
 *      https://osu.ppy.sh/wiki/en/Client/File_formats/Osu_(file_format)
 */

typedef float seconds_t;
typedef float percentage_t; // 1.0 is 100%

typedef struct {
    seconds_t start_time;
    seconds_t end_time;
} break_event_t;

// There's no concept of inherited and uninherited timing points
typedef struct {
    seconds_t       start_time;
    seconds_t       end_time;
    seconds_t       beat_length;
    float           SV;
    float           meter;
    percentage_t    volume;
    bool            kiai_enabled;
} timing_point_t;

typedef struct {
    seconds_t   start_time;
    seconds_t   end_time; // nonzero for hold note
    int         column;
} hitobject_t;

typedef enum {
    PLAYFIELD_EVENT_INVALID,
    PLAYFIELD_EVENT_CLICK,
    PLAYFIELD_EVENT_HOLD_START,
    PLAYFIELD_EVENT_HOLD_END,
} playfield_event_type_t;

typedef struct {
    playfield_event_type_t  type;
    seconds_t               time;
    float                   y;
} playfield_event_t;

typedef struct {
    kvec_t(playfield_event_t) events;
} playfield_column_t;

typedef struct {
    kvec_t(playfield_column_t) columns;
} playfield_t;

typedef struct {
    id_t        id;
    char        name[STRSIZE];
    char        url[STRSIZE];

    Music       audio;
    seconds_t   audio_length;
    char        audio_filename[STRSIZE];
    seconds_t   audio_leadin;
    seconds_t   preview_time;
    id_t        sample_set;
    bool        epilepsy_warning_enabled;
    bool        special_style_enabled;

    float HP;
    float CS; // eg. column count
    float OD;
    float AR;
    float SV;
    float STR;
    float star_rating;

    Image                   background;
    char                    background_filename[STRSIZE];
    char                    video_filename[STRSIZE];
    kvec_t(break_event_t)   breaks;
    kvec_t(timing_point_t)  timing_points;

    kvec_t(hitobject_t)     hitobjects;
    playfield_t             playfield;
} difficulty_t;

typedef struct {
    id_t        id;
    char        title[STRSIZE];
    char        title_unicode[STRSIZE];
    char        artist[STRSIZE];
    char        artist_unicode[STRSIZE];
    char        creator[STRSIZE];
    char        sources[STRSIZE];
    char        tags[STRSIZE];

    kvec_t(difficulty_t) difficulties;
} beatmap_t;


bool beatmap_load(beatmap_t* beatmap, const char* path); // directory or .osz
void beatmap_destroy(beatmap_t* beatmap);
void beatmap_debug_print(beatmap_t* beatmap);

timing_point_t*     difficulty_get_timing_point_for(difficulty_t* difficulty, seconds_t time);
playfield_event_t*  difficulty_get_playfield_event_for(difficulty_t* difficulty, seconds_t time, int column, bool find_nearest);


#endif

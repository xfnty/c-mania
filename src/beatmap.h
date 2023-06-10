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


/* enums */
typedef enum {
    PLAYFIELD_EVENT_INVALID,
    PLAYFIELD_EVENT_CLICK,
    PLAYFIELD_EVENT_HOLD_START,
    PLAYFIELD_EVENT_HOLD_END,
} playfield_event_type_t;


/* types */
typedef float seconds_t;
typedef float percentage_t;  // 1.0 is 100%

typedef struct {
    seconds_t start_time;
    seconds_t end_time;
} break_event_t;

typedef struct {  // There's no concept of inherited and uninherited timing points
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
    seconds_t   end_time;  // nonzero for hold note
    int         column;
} hitobject_t;

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
    seconds_t   audio_lead_in;
    seconds_t   preview_time;

    float HP;
    float CS; // eg. column count
    float OD;
    float AR;
    float SV;
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


/* function declarations */
bool beatmap_load(beatmap_t* beatmap, const char* path);
void beatmap_destroy(beatmap_t* beatmap);
void beatmap_debug_print(beatmap_t* beatmap);
timing_point_t*     difficulty_get_timing_point_for(difficulty_t* difficulty, seconds_t time);
playfield_event_t*  difficulty_get_playfield_event_for(difficulty_t* difficulty, seconds_t time, int column, bool find_nearest);


#endif

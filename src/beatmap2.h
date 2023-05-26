#ifndef BEATMAP_H
#define BEATMAP_H

#ifndef BEATMAP_STRING_SIZE
#define BEATMAP_STRING_SIZE 256
#endif

#include <stdbool.h>

#include <kvec.h>


typedef enum {
    BEATMAP_GAMEMODE_INVALID,
    BEATMAP_GAMEMODE_STANDART,
    BEATMAP_GAMEMODE_CATCH,
    BEATMAP_GAMEMODE_TAIKO,
    BEATMAP_GAMEMODE_MANIA,
} beatmap_gamemode_id_t;

// Source: https://osu.ppy.sh/wiki/en/Gameplay/Game_modifier
typedef enum {
    BEATMAP_MODE_NONE,  // No Mod
    BEATMAP_MODE_EZ,    // Easy
    BEATMAP_MODE_NF,    // No Fail
    BEATMAP_MODE_HT,    // Half Time
    BEATMAP_MODE_HR,    // Hard Rock
    BEATMAP_MODE_SD,    // Sudden Death
    BEATMAP_MODE_DT,    // Double Time
    BEATMAP_MODE_HD,    // Hidden
    BEATMAP_MODE_FI,    // Fade-in
    BEATMAP_MODE_RD,    // Random
    BEATMAP_MODE_AT,    // Auto
} beatmap_mode_id_t;

typedef enum {
    BEATMAP_EVENT_BACKGROUND = 0,
    BEATMAP_EVENT_VIDEO = 1,
    BEATMAP_EVENT_BREAK = 2,
} beatmap_event_id_t;

typedef struct beatmap_mode_s {
    const char*         name;
    const char*         abbreviation;
    const char*         description;
} beatmap_mode_t;

typedef struct beatmap_event_s {
    beatmap_event_id_t  type;

    union {
        struct {
            float   time_start;
            char    filename[BEATMAP_STRING_SIZE];
            int     offset_x;
            int     offset_y;
        } background;
        struct {
            float   time_start;
            char    filename[BEATMAP_STRING_SIZE];
            int     offset_x;
            int     offset_y;
        } video;
        struct {
            float   time_start;
            float   time_end;
        } breakpoint;
    };
} beatmap_event_t;

typedef enum {
    BEATMAP_EFFECT_NONE             = 0,
    BEATMAP_EFFECT_KIAI             = 0b00000001,
    BEATMAP_EFFECT_BARLINE_OMITTED  = 0b00000100,
} beatmap_event_effects_t;

typedef struct beatmap_timing_point_s {
    float                   time_start; // seconds
    float                   beat_length;
    int                     meter;
    int                     sample_set;
    int                     sample_index;
    float                   volume;       // [0, 1]
    bool                    is_uninherited;
    beatmap_event_effects_t effects;
} beatmap_timing_point_t;

typedef struct beatmap_s {
    int                     id;
    char                    audio_filepath[BEATMAP_STRING_SIZE];
    float                   audio_lead_in;

    float                   preview_time;
    float                   stack_leniency;
    beatmap_gamemode_id_t   mode;

    char                    name[BEATMAP_STRING_SIZE];

    float HP;
    float CS; // column count in mania
    float OD;
    float AR;
    float SV;
    float STR;

    kvec_t(beatmap_event_t)         events;
    kvec_t(beatmap_timing_point_t)  timing_points;
} beatmap_t;

typedef struct beatmapset_s {
    int id;
    char filepath[BEATMAP_STRING_SIZE];

    char title[BEATMAP_STRING_SIZE];
    char artist[BEATMAP_STRING_SIZE];
    char creator[BEATMAP_STRING_SIZE];

    kvec_t(beatmap_t) beatmaps;
} beatmapset_t;



beatmap_mode_t beatmap_mode_get(beatmap_mode_id_t id);


#endif

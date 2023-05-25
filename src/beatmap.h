#ifndef BEATMAP_H
#define BEATMAP_H

#include <stdbool.h>

#include <kvec.h>


// .osu format: https://osu.ppy.sh/wiki/en/Client/File_formats/Osu_(file_format)

typedef struct beatmap_break_s {
    int time_start;
    int time_end;
} beatmap_break_t;

typedef struct beatmap_timing_point_s {
    int     time_start;
    float   length;
    int     meter;
    bool    is_uninherited;
} beatmap_timing_point_t;

typedef struct beatmap_note_s {
    int     time_start;
    int     time_end;
    int     column;
    bool    is_hold_note;
} beatmap_note_t;

typedef struct beatmap_s {
    char format_version[10];

    // [General]
    char beatmap_filepath[256];
    char audio_filename[256];
    float       audio_length;
    float       audio_lead_in;
    int         preview_time;
    float       stack_leniency;
    int         mode;

    // [Metadata]
    char title[256];
    char artist[256];
    char creator[256];
    char difname[256];

    // [Difficulty]
    float HP;
    float CS; // column count in mania
    float OD;
    float AR;
    float SV;
    float STR;

    // [Events]
    kvec_t(beatmap_break_t) breaks;
    char background_filename[256];

    // [TimingPoints]
    kvec_t(beatmap_timing_point_t) timing_points;

    // [HitObjects]
    kvec_t(beatmap_note_t) notes;
} beatmap_t;


bool beatmap_load(const char* filepath, beatmap_t* new_beatmap, bool load_only_meta);
void beatmap_destroy(beatmap_t* beatmap);
void beatmap_debug_print(beatmap_t* beatmap);

#endif

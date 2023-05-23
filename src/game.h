#ifndef GAME_H
#define GAME_H

#include <stdbool.h>

#include <graphics.h>


typedef struct game_s {
    bool is_running;
    graphics_t graphics;
} game_t;


bool game_init(game_t* game);
void game_update(game_t* game);
void game_shutdown(game_t* game);


#endif
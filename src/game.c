#include "raylib.h"
#include <game.h>

#include <assert.h>

#define SCOPE_NAME "game"
#include <logging.h>
#include <graphics.h>

#define INIT_SYSTEM(ok) \
    do {\
        if(!(ok)) return false;\
    } while (0);


bool game_init(game_t* game) {
    INIT_SYSTEM(logging_init());
    INIT_SYSTEM(graphics_init(&game->graphics));

    game->is_running = true;
    return true;
}

void game_update(game_t* game) {
    BeginDrawing();
    ClearBackground(BLACK);
    EndDrawing();

    game->is_running = !WindowShouldClose();
}

void game_shutdown(game_t* game) {
    graphics_shutdown(&game->graphics);
    logging_shutdown();
}

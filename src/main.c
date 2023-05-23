#include <assert.h>
#include <stdbool.h>

#include <game.h>


int main(int argc, char const *argv[]) {
    game_t game;

    bool ok = game_init(&game);
    while (game.is_running)
        game_update(&game);
    game_shutdown(&game);

    return ok ? 0 : -1;
}

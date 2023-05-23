#include <raylib.h>
#include <raymath.h>

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <sys/types.h>

#include <kvec.h>

#include <logging.h>

#define pair_t(T1, T2) struct { T1 first; T2 second; }


typedef struct {
    const char* window_title;
    Vector2     window_size;

    const char* assets_folder;
    const char* user_folder;
    const char* start_map;

    const char* username;

    kvec_t(pair_t(char, char)) input_bindings;
} config_t;

typedef struct {
    char        keyboard_prev[512];
    char        keyboard[512];
    Vector2     mouse_pos;
    Vector2     mouse_delta;
} input_t;

typedef struct {
    Vector3 position;
    Vector3 rotation;
} transform_t;

typedef struct {
    kvec_t(Vector3) verticies;
} mesh3D_t;

typedef struct {
    mesh3D_t mesh;
} collider3D_t;

typedef struct {
    float   mass;
    Vector3 velocity;
    Vector3 acceleration;
    float   drag;
} rigidBody_t;

typedef struct {
    id_t            id;
    transform_t     transform;
    mesh3D_t        geometry;
    collider3D_t    collider;
    rigidBody_t     rigidbody;
} gameObject_t;

typedef struct {
    kvec_t(gameObject_t)    objects;
    mesh3D_t                static_geometry;
    collider3D_t            static_collider;
} world_t;

typedef struct {
    config_t    config;
    input_t     input;
    world_t     world;

    bool        was_exit_requested;
} game_t;


// void input_init(config_t* config);
// void graphics_init(config_t* config);
// void world_init(world_t* world, config_t* config);
// void logic_init(game_t* game);

// void input_update(input_t* input);
// void logic_update(game_t* game);
// void world_update_pending(world_t* world);
// void physics_update(world_t* world);
// void graphics_render(world_t* world);

int main(int argc, char const *argv[])
{
    // game_t game = {
    //     .config = {0},
    //     .input = {0},
    //     .world = {0},
    //     .was_exit_requested = false
    // };

    // graphics_init(&game.config);
    // input_init(&game.config);
    // world_init(&game.world, &game.config);
    // logic_init(&game);

    // while (!game.was_exit_requested) {
    //     input_update(&game.input);
    //     logic_update(&game);
    //     world_update_pending(&game.world);
    //     physics_update(&game.world);
    //     graphics_render(&game.world);
    // }

    // // shutdown
    // // ...

    return 0;
}

// void world_load_map(world_t* world, const char* map_path);

// void world_init(world_t* world, config_t* config) {
//     world_load_map(world, config->start_map);
// }

// void logic_update(game_t* game) {
//     if (game->input.keyboard[KEY_ESCAPE]) {
//         world_load_map(&game->world, game->config.start_map);
//     }

//     if (game->input.keyboard[KEY_LEFT_ALT] && game->input.keyboard[KEY_F4]) {
//         game->was_exit_requested = true;
//     }
// }

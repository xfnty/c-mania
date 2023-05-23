#include <graphics.h>

#include <assert.h>

#include <raylib.h>

#define SCOPE_NAME "graphics"
#include <logging.h>
#include <defines.h>


bool graphics_init(graphics_t* graphics) {
    assert(graphics != NULL);
    assert(graphics->_is_initialized == false);

    LOG("initializing ...");

    InitWindow(640, 480, "Game");
    SetTargetFPS(60);

    graphics->_is_initialized = IsWindowReady();
    return graphics->_is_initialized;
}

void graphics_draw(graphics_t* graphics, void* world) {
    assert(graphics != NULL);
    assert(world != NULL);
    assert(graphics->_is_initialized == true);
}

void graphics_shutdown(graphics_t* graphics) {
    assert(graphics != NULL);

    if (graphics->_is_initialized) {
        LOG("shutting down ...");
        CloseWindow();
        graphics->_is_initialized = false;
    }
}

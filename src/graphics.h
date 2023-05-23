#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <stdbool.h>


typedef struct graphics_s {
    bool _is_initialized;
} graphics_t;


bool graphics_init(graphics_t* graphics);
void graphics_draw(graphics_t* graphics, void* world);
void graphics_shutdown(graphics_t* graphics);


#endif

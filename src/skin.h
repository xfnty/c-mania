#ifndef SKIN_H
#define SKIN_H

#include <defines.h>


typedef struct skin_s {
    // [General]
    const char* name;
    const char* author;
    const char* version;
} skin_t;


bool skin_load(const char* filepath, skin_t* skin);
void skin_destroy(skin_t* skin);
void skin_debug_print(skin_t* skin);


#endif

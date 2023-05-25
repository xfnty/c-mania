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
#include <defines.h>
#include <skin.h>


int main(int argc, char const *argv[])
{
    logging_init();

    skin_t skin;
    if (skin_load("./assets/skin.ini", &skin)) {
        skin_debug_print(&skin);
        skin_destroy(&skin);
    }
    else {
        LOG("Failed");
    }

    logging_shutdown();
    return 0;
}

#include <skin.h>

#include <string.h>

#include <ini.h>

#include <logging.h>


static int handler2(void* user, const char* section, const char* name, const char* value, int lineno) {
    skin_t* skin = (skin_t*)user;
    if (strcmp(name, "Name") == 0) {
        skin->name = strdup(value);
    }
    else if (strcmp(name, "Author") == 0) {
        skin->author = strdup(value);
    }
    else if (strcmp(name, "Version") == 0) {
        skin->version = strdup(value);
    }
    // LOGF("%s: %s = %s", section, name, value);
    return true;
}

bool skin_load(const char* filepath, skin_t* skin) {
    return ini_parse(filepath, handler2, skin) == 0;
}

void skin_destroy(skin_t* skin) {
    free((void*)skin->name);
    free((void*)skin->author);
    free((void*)skin->version);
}

void skin_debug_print(skin_t* skin) {
    LOGF(
        "Skin:\n"
        "\tname: %s\n"
        "\tauthor: %s\n"
        "\tversion: %s",
        skin->name,
        skin->author,
        skin->version
    );
}

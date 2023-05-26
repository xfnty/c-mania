#include <beatmap2.h>

#include <assert.h>

#include <defines.h>
#define SCOPE_NAME "beatmap utils"
#include <logging.h>


static const beatmap_mode_t s_modes[] = {
    {
        "NoMod",
        "NM",
        "No mods are selected."
    },
    {
        "Easy",
        "EZ",
        "Reduces overall difficulty - more forgiving HP drain, less accuracy required."
    },
    {
        "No Fail",
        "NF",
        "You can't fail. No matter what."
    },
    {
        "Half Time",
        "HT",
        "Less zoom."
    },
    {
        "Hard Rock",
        "HR",
        "Everything just got a bit harder..."
    },
    {
        "Sudden Death",
        "SD",
        "Miss a note and fail."
    },
    {
        "Double Time",
        "DT",
        "Zoooooooooom."
    },
    {
        "Hidden",
        "HD",
        "The notes fade out before you hit them!"
    },
    {
        "Fade In",
        "FI",
        "The notes fade in as they approach the judgement bar."
    },
    {
        "Random",
        "RD",
        "Shuffle around the notes!"
    },
    {
        "Auto",
        "AT",
        "Watch a perfect automated play through the song."
    }
};


beatmap_mode_t beatmap_mode_get(beatmap_mode_id_t id) {
    assert(IS_OUT_OF_BOUNDS(id, s_modes));
    return s_modes[id];
}

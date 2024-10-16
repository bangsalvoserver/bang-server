#include "durations.h"

namespace banggame {

    using namespace std::chrono_literals;

    const durations_t durations {
        .move_cube =       133ms,
        .move_cubes =      250ms,
        .move_card =       333ms,
        .move_deck =       333ms,
        .deck_shuffle =    1333ms,
        .flip_card =       167ms,
        .tap_card =        167ms,
        .flash_card =      167ms,
        .short_pause =     333ms,
        .move_player =     1000ms,
        .player_hp =       333ms,
        .move_train =      500ms,
        .move_fame =       250ms
    };

}
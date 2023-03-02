#include "game/durations.h"

namespace banggame::durations {

    using namespace std::chrono_literals;

    duration_type move_cube =       133ms;
    duration_type move_cubes =      250ms;
    duration_type move_card =       333ms;
    duration_type deck_shuffle =    1333ms;
    duration_type flip_card =       167ms;
    duration_type tap_card =        167ms;
    duration_type flash_card =      167ms;
    duration_type short_pause =     333ms;
    duration_type move_player =     1000ms;
    duration_type player_hp =       333ms;
    duration_type move_train =      500ms;

}
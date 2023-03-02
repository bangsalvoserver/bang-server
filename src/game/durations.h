#ifndef __DURATIONS_H__
#define __DURATIONS_H__

#include <chrono>

namespace banggame::durations {

    using duration_type = const std::chrono::milliseconds;

    extern duration_type move_cube;
    extern duration_type move_cubes;
    extern duration_type move_card;
    extern duration_type deck_shuffle;
    extern duration_type flip_card;
    extern duration_type tap_card;
    extern duration_type flash_card;
    extern duration_type short_pause;
    extern duration_type move_player;
    extern duration_type player_hp;
    extern duration_type move_train;

}

#endif
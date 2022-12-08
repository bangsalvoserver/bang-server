#ifndef __DURATIONS_H__
#define __DURATIONS_H__

#include <chrono>

namespace banggame::durations {

    extern const std::chrono::milliseconds move_cube;
    extern const std::chrono::milliseconds move_cubes;
    extern const std::chrono::milliseconds move_card;
    extern const std::chrono::milliseconds deck_shuffle;
    extern const std::chrono::milliseconds flip_card;
    extern const std::chrono::milliseconds tap_card;
    extern const std::chrono::milliseconds flash_card;
    extern const std::chrono::milliseconds short_pause;
    extern const std::chrono::milliseconds move_player;
    extern const std::chrono::milliseconds player_hp;

}

#endif
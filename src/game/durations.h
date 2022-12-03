#ifndef __DURATIONS_H__
#define __DURATIONS_H__

#include "game_update.h"

namespace banggame {
    template<game_update_type E> constexpr ticks update_duration{0};

    #define DURATION(name, value) \
    template<> constexpr ticks update_duration<game_update_type::name> = std::chrono::duration_cast<ticks>(value);

    DURATION(move_card, 333ms)
    DURATION(move_cubes, 250ms)
    DURATION(move_scenario_deck, 333ms)
    DURATION(deck_shuffled, 1333ms)
    DURATION(show_card, 167ms)
    DURATION(hide_card, 167ms)
    DURATION(tap_card, 167ms)
    DURATION(flash_card, 167ms)
    DURATION(short_pause, 333ms)
    DURATION(player_remove, 1000ms)
    DURATION(player_hp, 333ms)
    DURATION(player_show_role, 167ms)

    #undef DURATION
}

#endif
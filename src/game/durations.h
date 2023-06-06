#ifndef __DURATIONS_H__
#define __DURATIONS_H__

#include <chrono>

namespace banggame {

    using game_duration = std::chrono::milliseconds;

    extern const struct durations_t {
        game_duration move_cube;
        game_duration move_cubes;
        game_duration move_card;
        game_duration move_deck;
        game_duration deck_shuffle;
        game_duration flip_card;
        game_duration tap_card;
        game_duration flash_card;
        game_duration short_pause;
        game_duration move_player;
        game_duration player_hp;
        game_duration move_train;
    } durations;

}

#endif
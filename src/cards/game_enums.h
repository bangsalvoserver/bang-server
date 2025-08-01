#ifndef __CARDS_GAME_ENUMS_H__
#define __CARDS_GAME_ENUMS_H__

#include "utils/enums.h"

namespace banggame {

    enum class sound_id {
        gamestart,
        victory,
        draw,
        death,
        bang,
        gatling,
        indians,
        duel,
        dynamite,
        generalstore,
        shuffle,
        bandidos,
        snake,
        train,
    };
    
    enum class effect_flag {
        is_bang,
        is_missed,
        play_as_bang,
        single_target,
        multi_target,
        target_players,
    };
    
    enum class game_flag {
        game_over,
        invert_rotation,
        disable_equipping,
        phase_one_draw_discard,
        phase_one_override,
        disable_player_distances,
        showdown,
        hands_shown,
        free_for_all,
    };

    enum class player_flag {
        dead,
        ghost_1,
        ghost_2,
        temp_ghost,
        shadow,
        extra_turn,
        stick_of_dynamite,
        treat_missed_as_bang,
        treat_any_as_bang,
        treat_any_as_missed,
        ignore_distances,
        role_revealed,
        show_hand_playing,
        skip_turn,
        legend,
        removed,
        winner,
    };
}

#endif
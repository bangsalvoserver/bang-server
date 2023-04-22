#ifndef __CARDS_GAME_ENUMS_H__
#define __CARDS_GAME_ENUMS_H__

#include "utils/enums.h"

namespace banggame {
    
    DEFINE_ENUM_FLAGS(effect_flags,
        (is_bang)
        (play_as_bang)
        (escapable)
        (single_target)
        (multi_target)
        (deflected)
    )
    
    DEFINE_ENUM_FLAGS(game_flags,
        (game_over)
        (invert_rotation)
        (disable_equipping)
        (phase_one_draw_discard)
        (phase_one_override)
        (disable_player_distances)
        (treat_any_as_bang)
        (hands_shown)
        (free_for_all)
    )

    DEFINE_ENUM_FLAGS(player_flags,
        (dead)
        (ghost_1)
        (ghost_2)
        (temp_ghost)
        (extra_turn)
        (treat_missed_as_bang)
        (role_revealed)
        (skip_turn)
        (removed)
        (winner)
    )
    
    DEFINE_ENUM(discard_all_reason,
        (death)
        (sheriff_killed_deputy)
        (disable_temp_ghost)
        (discard_ghost)
    )
}

#endif
#ifndef __ARMEDANDDANGEROUS_CUBE_SLOTS_H__
#define __ARMEDANDDANGEROUS_CUBE_SLOTS_H__

#include "game/card.h"
#include "game/player.h"

namespace banggame {

    inline auto cube_slots(const_player_ptr target) {
        return rv::concat(
            target->m_characters | rv::take(1),
            target->m_table | rv::filter(&card::is_orange)
        );
    }

    inline int count_cubes(const_player_ptr target) {
        return rn::accumulate(cube_slots(target)
            | rv::transform(&card::num_cubes), 0);
    }
}

#endif
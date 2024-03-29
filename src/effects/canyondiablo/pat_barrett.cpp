#include "pat_barrett.h"

#include "game/game.h"

namespace banggame {

    void equip_pat_barrett::on_enable(card *target_card, player *target) {
        target->m_game->add_listener<event_type::count_range_mod>(target_card, [=](const player *origin, range_mod_type type, int &value) {
            if (origin == target && type == range_mod_type::distance_mod) {
                value += origin->m_max_hp - origin->m_hp;
            }
        });
    }
}
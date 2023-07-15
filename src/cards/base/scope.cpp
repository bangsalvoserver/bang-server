#include "scope.h"

#include "game/game.h"

namespace banggame {

    void equip_scope::on_enable(card *target_card, player *target) {
        target->m_game->add_event<event_type::count_range_mod>(target_card, [=](const player *origin, range_mod_type type, int &value) {
            if (origin == target && type == range_mod_type::range_mod) {
                ++value;
            }
        });
    }
}
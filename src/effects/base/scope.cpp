#include "scope.h"

#include "cards/game_events.h"

#include "game/game_table.h"

namespace banggame {

    void equip_scope::on_enable(card_ptr target_card, player_ptr target) {
        target->m_game->add_listener<event_type::count_range_mod>(target_card, [=](const_player_ptr origin, range_mod_type type, int &value) {
            if (origin == target && type == range_mod_type::range_mod) {
                ++value;
            }
        });
    }
}
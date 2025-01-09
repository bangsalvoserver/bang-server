#include "volcanic.h"

#include "game/game_table.h"

#include "bang.h"

namespace banggame {
    
    void equip_volcanic::on_enable(card_ptr target_card, player_ptr target) {
        target->m_game->add_listener<event_type::count_bangs_played>(target_card, [=](const_player_ptr origin, int &value) {
            if (origin == target) {
                value = 0;
            }
        });
    }

    void equip_volcanic::on_disable(card_ptr target_card, player_ptr target) {
        target->m_game->remove_listeners(event_card_key{target_card, 0});
    }
}
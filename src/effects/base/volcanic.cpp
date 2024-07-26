#include "volcanic.h"

#include "game/game.h"

#include "bang.h"

namespace banggame {
    
    void equip_volcanic::on_enable(card_ptr target_card, player_ptr target) {
        target->m_game->add_listener<event_type::count_bangs_played>(target_card, [=](player_ptr origin, int &value) {
            if (origin == target) {
                value = 0;
            }
        });
    }
}
#include "lemonade_jim.h"

#include "effects/base/beer.h"
#include "effects/base/can_play_card.h"

#include "game/game.h"

namespace banggame {
    
    void equip_lemonade_jim::on_enable(card_ptr target_card, player_ptr origin) {
        origin->m_game->add_listener<event_type::on_play_beer>(target_card, [=](player_ptr target, bool is_sold) {
            if (origin != target) {
                target->m_game->queue_request<request_can_play_card>(target_card, target, origin);
            }
        });
    }
}
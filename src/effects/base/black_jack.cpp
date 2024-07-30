#include "black_jack.h"
#include "draw.h"

#include "game/game.h"

namespace banggame {
    
    void equip_black_jack::on_enable(card_ptr target_card, player_ptr target) {
        target->m_game->add_listener<event_type::on_card_drawn>(target_card, [target, target_card](player_ptr origin, card_ptr drawn_card, shared_request_draw req_draw, bool &reveal) {
            if (origin == target && req_draw->get_drawn_cards() == 2) {
                reveal = true;

                if (target->m_game->get_card_sign(drawn_card).is_red()) {
                    req_draw->add_cards_to_draw(1);
                }
            }
        });
    }
}
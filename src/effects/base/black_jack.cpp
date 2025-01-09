#include "black_jack.h"
#include "draw.h"

#include "game/game_table.h"

namespace banggame {
    
    void equip_black_jack::on_enable(card_ptr target_card, player_ptr target) {
        target->m_game->add_listener<event_type::on_card_drawn>(target_card, [target, target_card](player_ptr origin, card_ptr drawn_card, shared_request_draw req_draw, bool &reveal) {
            if (origin == target && req_draw->num_drawn_cards == 2) {
                reveal = true;

                if (drawn_card->get_modified_sign().is_red()) {
                    ++req_draw->num_cards_to_draw;
                }
            }
        });
    }
}
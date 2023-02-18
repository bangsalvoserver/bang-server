#include "black_jack.h"

#include "game/game.h"

namespace banggame {
    
    void equip_black_jack::on_enable(card *target_card, player *target) {
        target->m_game->add_listener<event_type::on_card_drawn>(target_card, [target, target_card](player *origin, card *drawn_card, bool &reveal) {
            if (origin == target && origin->m_num_drawn_cards == 2) {
                reveal = true;

                if (target->m_game->get_card_sign(drawn_card).is_red()) {
                    origin->m_game->queue_action([=]{
                        origin->add_to_hand_phase_one(origin->m_game->phase_one_drawn_card());
                    }, 2);
                }
            }
        });
    }
}
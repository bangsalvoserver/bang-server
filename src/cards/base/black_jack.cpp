#include "black_jack.h"

#include "game/game.h"

namespace banggame {
    
    void effect_black_jack::on_enable(card *target_card, player *target) {
        target->m_game->add_listener<event_type::on_card_drawn>(target_card, [target, target_card](player *origin, card *drawn_card, bool &reveal) {
            if (origin == target && origin->m_num_drawn_cards == 2) {
                reveal = true;

                card_suit suit = target->get_card_sign(drawn_card).suit;
                if (suit == card_suit::hearts || suit == card_suit::diamonds) {
                    event_card_key key{target_card, 2};
                    origin->m_game->add_listener<event_type::post_draw_cards>(key, [=](player *p) {
                        if (p == origin) {
                            origin->add_to_hand_phase_one(origin->m_game->phase_one_drawn_card());
                            origin->m_game->remove_listeners(key);
                        }
                    });
                }
            }
        });
    }
}
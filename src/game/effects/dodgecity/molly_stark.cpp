#include "molly_stark.h"

#include "../../game.h"

namespace banggame {

    void effect_molly_stark::on_enable(card *target_card, player *p) {
        p->m_game->add_listener<event_type::on_play_hand_card>(target_card, [p, target_card](player *target, card *card) {
            if (p == target && p->m_game->m_playing != p) {
                p->m_game->queue_action([p, target_card]{
                    if (p->alive()) {                        
                        p->m_game->flash_card(target_card);
                        p->draw_card(1, target_card);
                    }
                });
            }
        });
    }
}
#include "el_gringo.h"

#include "../../game.h"

namespace banggame {
    
    void effect_el_gringo::on_enable(card *target_card, player *p) {
        p->m_game->add_listener<event_type::after_hit>({target_card, 2}, [=](card *origin_card, player *origin, player *target, int damage, effect_flags flags) {
            if (origin && p == target) {
                origin->m_game->queue_action([=]{
                    if (origin->alive() && p->m_game->m_playing != p) {
                        for (int i=0; i<damage && !origin->m_hand.empty(); ++i) {
                            target->m_game->flash_card(target_card);
                            card *stolen_card = origin->random_hand_card();
                            target->m_game->add_log(update_target::includes(origin, target), "LOG_STOLEN_CARD", target, origin, stolen_card);
                            target->m_game->add_log(update_target::excludes(origin, target), "LOG_STOLEN_CARD_FROM_HAND", target, origin);
                            target->steal_card(stolen_card);
                            target->m_game->call_event<event_type::on_effect_end>(p, target_card);
                        }
                    }
                });
            }
        });
    }
}
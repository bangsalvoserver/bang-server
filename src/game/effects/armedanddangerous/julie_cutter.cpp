#include "julie_cutter.h"

#include "../../game.h"
#include "../base/bang.h"

namespace banggame {
    
    void effect_julie_cutter::on_enable(card *target_card, player *p) {
        p->m_game->add_listener<event_type::after_hit>(target_card, [=](card *origin_card, player *origin, player *target, int damage, bool is_bang) {
            if (origin && p == target && origin != target) {
                origin->m_game->queue_action([=]{
                    if (origin->alive()) {
                        target->m_game->draw_check_then(target, target_card, [=](card *drawn_card) {
                            card_suit suit = target->get_card_sign(drawn_card).suit;
                            if (suit == card_suit::hearts || suit == card_suit::diamonds) {
                                target->m_game->add_log("LOG_CARD_HAS_EFFECT", target_card);
                                target->m_game->queue_request<request_bang>(target_card, target, origin);
                            }
                        });
                    }
                });
            }
        });
    }
}
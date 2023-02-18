#include "julie_cutter.h"

#include "game/game.h"
#include "cards/base/bang.h"

namespace banggame {
    
    void equip_julie_cutter::on_enable(card *target_card, player *p) {
        p->m_game->add_listener<event_type::on_hit>(target_card, [=](card *origin_card, player *origin, player *target, int damage, effect_flags flags) {
            if (origin && p == target && origin != target) {
                origin->m_game->queue_action([=]{
                    if (target->alive()) {
                        target->m_game->draw_check_then(target, target_card, &card_sign::is_red, [=](bool result) {
                            if (result) {
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
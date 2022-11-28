#include "colorado_bill.h"

#include "game/game.h"
#include "cards/base/bang.h"

namespace banggame {
    
    void equip_colorado_bill::on_enable(card *target_card, player *p) {
        p->m_game->add_listener<event_type::apply_bang_modifier>(target_card, [=](player *origin, request_bang *req) {
            if (p == origin) {
                origin->m_game->draw_check_then(origin, target_card, [=](card_sign sign) {
                    if (sign.suit == card_suit::spades) {
                        p->m_game->add_log("LOG_CARD_HAS_EFFECT", target_card);
                        req->unavoidable = true;
                    }
                });
            }
        });
    }
}
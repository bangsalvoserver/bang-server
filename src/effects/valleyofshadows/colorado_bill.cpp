#include "colorado_bill.h"

#include "game/game.h"
#include "effects/base/bang.h"
#include "effects/base/draw_check.h"

namespace banggame {
    
    void equip_colorado_bill::on_enable(card_ptr target_card, player_ptr p) {
        p->m_game->add_listener<event_type::apply_bang_modifier>(target_card, [=](player_ptr origin, shared_request_bang req) {
            if (p == origin) {
                origin->m_game->queue_request<request_check>(origin, target_card, &card_sign::is_spades, [=](bool result) {
                    if (result) {
                        p->m_game->add_log("LOG_CARD_HAS_EFFECT", target_card);
                        req->unavoidable = true;
                    }
                });
            }
        });
    }
}
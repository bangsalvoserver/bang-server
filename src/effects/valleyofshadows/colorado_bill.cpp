#include "colorado_bill.h"

#include "cards/game_enums.h"

#include "effects/base/bang.h"
#include "effects/base/draw_check.h"

#include "game/game_table.h"

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
    
    void equip_colorado_bill2::on_enable(card_ptr target_card, player_ptr p) {
        p->m_game->add_listener<event_type::on_missed>(target_card, [=](card_ptr origin_card, player_ptr origin, player_ptr target, card_ptr missed_card, effect_flags flags) {
            if (p == origin && flags.check(effect_flag::is_bang) && flags.check(effect_flag::is_missed)) {
                origin->m_game->queue_request<request_check>(origin, target_card, &card_sign::is_spades, [=](bool result) {
                    if (result) {
                        if (auto req = p->m_game->top_request<request_bang>()) {
                            p->m_game->add_log("LOG_CARD_HAS_EFFECT", target_card);
                            req->unavoidable = true;
                        }
                    }
                });
            }
        });
    }
}
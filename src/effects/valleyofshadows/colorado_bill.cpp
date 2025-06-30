#include "colorado_bill.h"

#include "cards/game_enums.h"

#include "effects/base/bang.h"
#include "effects/base/missed.h"
#include "effects/base/draw_check.h"

#include "game/game_table.h"

namespace banggame {
    
    void equip_colorado_bill::on_enable(card_ptr origin_card, player_ptr origin) {
        origin->m_game->add_listener<event_type::apply_bang_modifier>(origin_card, [=](player_ptr e_origin, shared_request_bang req) {
            if (e_origin == origin) {
                bool indifferent = req->target->empty_hand() && count_missed_cards(req->target) == 0;
                origin->m_game->queue_request<request_check>(origin, origin_card, [=](card_ptr target_card) {
                    return draw_check_result {
                        .lucky = get_modified_sign(target_card).is_spades(),
                        .indifferent = indifferent
                    };
                }, [=](bool result) {
                    if (result) {
                        origin->m_game->add_log("LOG_CARD_HAS_EFFECT", origin_card);
                        req->unavoidable = true;
                    }
                });
            }
        });
    }
    
    void equip_colorado_bill2::on_enable(card_ptr target_card, player_ptr origin) {
        origin->m_game->add_listener<event_type::on_missed>(target_card, [=](card_ptr missed_card, effect_flags flags, shared_request_bang req) {
            if (req->origin == origin && flags.check(effect_flag::is_bang) && flags.check(effect_flag::is_missed)) {
                origin->m_game->queue_request<request_check>(origin, target_card, &card_sign::is_spades, [=](bool result) {
                    if (result) {
                        origin->m_game->add_log("LOG_CARD_HAS_EFFECT", target_card);
                        req->unavoidable = true;
                    }
                });
            }
        });
    }
}
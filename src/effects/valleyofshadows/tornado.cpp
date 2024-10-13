#include "tornado.h"

#include "effects/base/pick.h"
#include "cards/game_enums.h"

#include "game/game.h"

namespace banggame {

    struct request_tornado : request_picking {
        request_tornado(card_ptr origin_card, player_ptr origin, player_ptr target, effect_flags flags = {})
            : request_picking(origin_card, origin, target, flags) {}
        
        bool can_pick(const_card_ptr target_card) const override {
            return target_card->pocket == pocket_type::player_hand && target_card->owner == target
                && !target->m_game->is_usage_disabled(target_card);
        }

        void on_update() override {
            if (target->immune_to(origin_card, origin, flags)) {
                target->m_game->pop_request();
            } else if (rn::none_of(target->m_hand, [&](const_card_ptr c) { return can_pick(c); })) {
                target->m_game->pop_request();
                target->reveal_hand();
                target->draw_card(2, origin_card);
            } else if (target->m_hand.size() == 1) {
                auto_pick();
            }
        }
        
        void on_pick(card_ptr target_card) override {
            target->m_game->pop_request();
            target->m_game->add_log("LOG_DISCARDED_CARD_FOR", origin_card, target, target_card);
            target->discard_used_card(target_card);
            target->draw_card(2, origin_card);
        }
        
        game_string status_text(player_ptr owner) const override {
            if (target == owner) {
                return {"STATUS_TORNADO", origin_card};
            } else {
                return {"STATUS_TORNADO_OTHER", target, origin_card};
            }
        }
    };
    
    void effect_tornado::on_play(card_ptr origin_card, player_ptr origin, player_ptr target, effect_flags flags) {
        target->m_game->queue_request<request_tornado>(origin_card, origin, target, flags);
    }
}
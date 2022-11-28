#include "tornado.h"

#include "game/game.h"

namespace banggame {

    struct request_tornado : request_base {
        request_tornado(card *origin_card, player *origin, player *target, effect_flags flags = {})
            : request_base(origin_card, origin, target, flags) {}
        
        bool can_pick(card *target_card) const override {
            return target_card->pocket == pocket_type::player_hand && target_card->owner == target;
        }

        bool auto_resolve() override {
            if (target->m_hand.empty()) {
                auto lock = target->m_game->lock_updates(true);
                target->draw_card(2, origin_card);
                return true;
            } else {
                return auto_pick();
            }
        }
        
        void on_pick(card *target_card) override {
            auto lock = target->m_game->lock_updates(true);
            target->m_game->add_log("LOG_DISCARDED_CARD_FOR", origin_card, target, target_card);
            target->discard_card(target_card);
            target->draw_card(2, origin_card);
        }
        
        game_string status_text(player *owner) const override {
            if (target == owner) {
                return {"STATUS_TORNADO", origin_card};
            } else {
                return {"STATUS_TORNADO_OTHER", target, origin_card};
            }
        }
    };
    
    void effect_tornado::on_play(card *origin_card, player *origin, player *target, effect_flags flags) {
        // ignore flags ... why would you ever play escape vs tornado?
        target->m_game->queue_request<request_tornado>(origin_card, origin, target);
    }
}
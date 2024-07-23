#include "tornado.h"

#include "effects/base/pick.h"

#include "game/game.h"

namespace banggame {

    struct request_tornado : request_picking {
        request_tornado(card *origin_card, player *origin, player *target, effect_flags flags = {})
            : request_picking(origin_card, origin, target, flags) {}
        
        bool can_pick(const card *target_card) const override {
            return target_card->pocket == pocket_type::player_hand && target_card->owner == target;
        }

        void on_update() override {
            if (target->immune_to(origin_card, origin, flags)) {
                target->m_game->pop_request();
            } else if (target->empty_hand()) {
                target->m_game->pop_request();
                target->draw_card(2, origin_card);
            } else {
                auto_pick();
            }
        }
        
        void on_pick(card *target_card) override {
            target->m_game->pop_request();
            target->m_game->add_log("LOG_DISCARDED_CARD_FOR", origin_card, target, target_card);
            target->discard_used_card(target_card);
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
        target->m_game->queue_request<request_tornado>(origin_card, origin, target, flags);
    }
}
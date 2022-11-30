#include "ricochet.h"

#include "game/game.h"
#include "cards/base/bang.h"
#include "cards/base/steal_destroy.h"

namespace banggame {
    
    struct request_ricochet : request_targeting, missable_request {
        using request_targeting::request_targeting;

        bool auto_resolve() override {
            return target->m_hand.empty() && auto_respond();
        }

        void on_resolve_target() override {
            effect_destroy{}.on_resolve(origin_card, origin, target_card);
        }

        void on_miss() override {
            target->m_game->pop_request();
        }

        game_string status_text(player *owner) const override {
            if (target == owner) {
                return {"STATUS_RICOCHET", origin_card, target_card};
            } else {
                return {"STATUS_RICOCHET_OTHER", target, origin_card, target_card};
            }
        }
    };

    void effect_ricochet::on_play(card *origin_card, player *origin, card *target_card) {
        origin->m_game->queue_request<request_ricochet>(origin_card, origin, target_card->owner, target_card);
    }
}
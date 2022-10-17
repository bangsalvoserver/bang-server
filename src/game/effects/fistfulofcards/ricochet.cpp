#include "ricochet.h"

#include "../../game.h"
#include "../base/bang.h"
#include "../base/steal_destroy.h"

namespace banggame {
    
    struct request_ricochet : request_targeting, resolvable_request, missable_request {
        using request_targeting::request_targeting;

        void on_resolve() override {
            origin->m_game->pop_request();
            effect_destroy{}.on_resolve(origin_card, origin, target_card);
            origin->m_game->update_request();
        }

        void on_miss() override {
            auto target = this->target;
            target->m_game->pop_request();
            target->m_game->update_request();
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
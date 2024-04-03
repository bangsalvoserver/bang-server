#include "shade_oconnor.h"

#include "cards/game_enums.h"

#include "game/game.h"
#include "ruleset.h"

#include "effects/base/resolve.h"

namespace banggame {
    
    struct request_shade_oconnor : request_resolvable {
        request_shade_oconnor(card *origin_card, player *target)
            : request_resolvable(origin_card, nullptr, target, {}, -1) {}

        void on_update() override {
            auto_resolve();
        }

        void on_resolve() override {
            target->m_game->pop_request();
        }
        
        game_string status_text(player *owner) const override {
            if (target == owner) {
                return {"STATUS_CAN_PLAY_CARD", origin_card};
            } else {
                return {"STATUS_CAN_PLAY_CARD_OTHER", target, origin_card};
            }
        }
    };
    
    void equip_shade_oconnor::on_enable(card *target_card, player *origin) {
        origin->m_game->add_listener<event_type::on_train_advance>({target_card, 1}, [=](player *target, shared_effect_context ctx) {
            if (origin != target) {
                origin->m_game->queue_request<request_shade_oconnor>(target_card, origin);
            }
        });
    }

    bool effect_shade_oconnor::can_play(card *origin_card, player *origin) {
        return origin->m_game->top_request<request_shade_oconnor>(origin) != nullptr;
    }

    void effect_shade_oconnor::on_play(card *origin_card, player *origin) {
        origin->m_game->pop_request();
    }
}
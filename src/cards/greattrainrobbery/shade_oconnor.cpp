#include "shade_oconnor.h"

#include "game/game.h"

namespace banggame {
    
    struct request_shade_oconnor : request_base {
        request_shade_oconnor(card *origin_card, player *origin, player *target)
            : request_base(origin_card, origin, target, effect_flags::auto_respond) {}
        
        game_string status_text(player *owner) const override {
            if (target == owner) {
                return {"STATUS_CAN_PLAY_CARD", origin_card};
            } else {
                return {"STATUS_CAN_PLAY_CARD_OTHER", target, origin_card};
            }
        }
    };
    
    void equip_shade_oconnor::on_enable(card *target_card, player *origin) {
        // TODO add_listener on_train_advance
        // if out of turn : queue_request request_shade_oconnor
    }

    bool effect_shade_oconnor::can_play(card *origin_card, player *origin) {
        return origin->m_game->top_request<request_shade_oconnor>(origin) != nullptr;
    }

    void effect_shade_oconnor::on_play(card *origin_card, player *origin) {
        origin->m_game->pop_request();
    }
}
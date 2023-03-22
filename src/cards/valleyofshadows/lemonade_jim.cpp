#include "lemonade_jim.h"

#include "cards/game_enums.h"

#include "game/game.h"

namespace banggame {

    struct request_lemonade_jim : request_auto_select {
        request_lemonade_jim(card *origin_card, player *origin, player *target)
            : request_auto_select(origin_card, origin, target) {}
        
        game_string status_text(player *owner) const override {
            if (target == owner) {
                return {"STATUS_CAN_PLAY_CARD", origin_card};
            } else {
                return {"STATUS_CAN_PLAY_CARD_OTHER", target, origin_card};
            }
        }
    };
    
    void equip_lemonade_jim::on_enable(card *target_card, player *origin) {
        origin->m_game->add_listener<event_type::on_play_beer>(target_card, [=](player *target) {
            if (origin != target) {
                if (!origin->empty_hand() && origin->m_hp < origin->m_max_hp) {
                    target->m_game->queue_request<request_lemonade_jim>(target_card, target, origin);
                }
            }
        });
    }

    bool effect_lemonade_jim::can_play(card *origin_card, player *origin) {
        return origin->m_game->top_request<request_lemonade_jim>(origin) != nullptr;
    }

    void effect_lemonade_jim::on_play(card *origin_card, player *origin) {
        origin->m_game->pop_request();
    }
}
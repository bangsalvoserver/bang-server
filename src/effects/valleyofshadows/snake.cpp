#include "snake.h"

#include "game/game.h"

#include "effects/base/predraw_check.h"

namespace banggame {
    
    void equip_snake::on_enable(card *target_card, player *target) {
        target->m_game->add_listener<event_type::on_predraw_check>(target_card, [=](player *p, card *e_card) {
            if (p == target && e_card == target_card) {
                target->m_game->queue_request<request_check>(target, target_card, std::not_fn(&card_sign::is_spades), [=](bool result) {
                    if (!result) {
                        target->m_game->add_log("LOG_CARD_HAS_EFFECT", target_card);
                        target->m_game->play_sound("snake");
                        target->damage(target_card, nullptr, 1);
                    }
                });
            }
        });
    }
}
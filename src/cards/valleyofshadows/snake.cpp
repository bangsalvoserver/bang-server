#include "snake.h"

#include "game/game.h"

namespace banggame {
    
    void equip_snake::on_enable(card *target_card, player *target) {
        target->m_game->add_listener<event_type::on_predraw_check>(target_card, [=](player *p, card *e_card) {
            if (p == target && e_card == target_card) {
                target->m_game->draw_check_then(target, target_card, [=](card_sign sign) {
                    if (sign.suit == card_suit::spades) {
                        target->m_game->add_log("LOG_CARD_HAS_EFFECT", target_card);
                        target->damage(target_card, nullptr, 1);
                    }
                });
            }
        });
    }
}
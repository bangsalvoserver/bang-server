#include "jail.h"

#include "game/game.h"

#include "effects/base/predraw_check.h"

namespace banggame {

    void equip_jail::on_enable(card *target_card, player *target) {
        target->m_game->add_listener<event_type::on_predraw_check>(target_card, [=](player *p, card *e_card) {
            if (p == target && e_card == target_card) {
                target->m_game->draw_check_then(target, target_card, &card_sign::is_hearts, [=](bool result) {
                    target->discard_card(target_card);
                    if (result) {
                        target->m_game->add_log("LOG_JAIL_BREAK", target);
                    } else {
                        target->m_game->add_log("LOG_SKIP_TURN", target);
                        target->skip_turn();
                    }
                });
            }
        });
    }
}
#include "dining_car.h"

#include "game/game.h"

#include "cards/base/predraw_check.h"

namespace banggame {

    void equip_dining_car::on_enable(card *origin_card, player *origin) {
        origin->m_game->add_listener<event_type::on_predraw_check>(origin_card, [=](player *target, card *target_card) {
            if (origin == target && origin_card == target_card) {
                origin->m_game->draw_check_then(origin, origin_card, &card_sign::is_hearts, [=](bool result) {
                    if (result) {
                        origin->heal(1);
                    }
                });
            }
        });
    }
}
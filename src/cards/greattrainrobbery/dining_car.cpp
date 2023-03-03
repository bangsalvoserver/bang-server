#include "dining_car.h"

#include "game/game.h"

namespace banggame {

    void equip_dining_car::on_enable(card *origin_card, player *origin) {
        origin->m_game->add_listener<event_type::on_turn_start>({origin_card, 3}, [=](player *target) {
            if (origin == target) {
                origin->m_game->draw_check_then(origin, origin_card, &card_sign::is_hearts, [=](bool result) {
                    if (result) {
                        origin->heal(1);
                    }
                });
            }
        });
    }
}
#include "private_car.h"

#include "game/game.h"
#include "cards/game_enums.h"

namespace banggame {

    void equip_private_car::on_enable(card *origin_card, player *origin) {
        origin->m_game->add_listener<event_type::check_card_target>(origin_card, [=](card *e_origin_card, player *e_origin, player *e_target, game_string &out_error) {
            if (e_origin_card && e_origin != e_target && e_target == origin && origin->empty_hand()) {
                out_error = {"ERROR_CANNOT_TARGET_PLAYER", origin_card, origin, e_origin_card};
            }
        });
    }
}
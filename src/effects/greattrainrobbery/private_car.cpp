#include "private_car.h"

#include "game/game.h"
#include "cards/game_enums.h"
#include "effects/base/bang.h"

namespace banggame {

    void equip_private_car::on_enable(card *origin_card, player *origin) {
        origin->m_game->add_listener<event_type::check_bang_target>(origin_card, [=](card *e_origin_card, player *e_origin, player *e_target, effect_flags flags, game_string &out_error) {
            if (e_origin_card && e_origin != e_target && e_target == origin && origin->empty_hand() && flags.check(effect_flag::is_bang)) {
                out_error = {"ERROR_CANNOT_TARGET_PLAYER", origin_card, origin, e_origin_card};
            }
        });
    }
}
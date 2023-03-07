#include "mick_defender.h"

#include "cards/game_enums.h"

#include "game/game.h"

namespace banggame {

    void equip_mick_defender::on_enable(card *origin_card, player *origin) {
        origin->m_game->add_listener<event_type::apply_escapable_modifier>({origin_card, -1},
            [=](card *e_origin_card, player *e_origin, const player *e_target, effect_flags flags, int &value) {
                if (e_target == origin && !origin->empty_hand() && bool(flags & effect_flags::escapable)) {
                    value = 2;
                }
            });
    }
}
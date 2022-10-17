#include "mick_defender.h"

#include "../../game.h"

namespace banggame {

    void effect_mick_defender::on_enable(card *origin_card, player *origin) {
        origin->m_game->add_listener<event_type::apply_escapable_modifier>({origin_card, 1}, [=](card *e_origin_card, player *e_origin, const player *e_target, effect_flags flags, bool &value) {
            value = value || bool(flags & effect_flags::escapable) && (origin == e_target);
        });
    }
}
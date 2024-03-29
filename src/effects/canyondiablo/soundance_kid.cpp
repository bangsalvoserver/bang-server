#include "soundance_kid.h"

#include "cards/game_enums.h"
#include "effects/base/damage.h"

#include "game/game.h"

namespace banggame {

    void equip_soundance_kid::on_enable(card *target_card, player *p) {
        p->m_game->add_listener<event_type::on_hit>({target_card, 5}, [=](card *origin_card, player *origin, player *target, int damage, effect_flags flags) {
            if (origin == p && bool(flags & effect_flags::is_bang)) {
                origin->m_game->flash_card(target_card);
                origin->draw_card(1, target_card);
            }
        });
    }
}
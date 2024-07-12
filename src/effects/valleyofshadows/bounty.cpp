#include "bounty.h"

#include "cards/game_enums.h"
#include "effects/base/damage.h"

#include "game/game.h"

namespace banggame {
    
    void equip_bounty::on_enable(card *target_card, player *p) {
        p->m_game->add_listener<event_type::on_hit>({target_card, 4}, [=](card *origin_card, player *origin, player *target, int damage, effect_flags flags) {
            if (origin && target == p && flags.check(effect_flag::is_bang)) {
                target_card->flash_card();
                origin->draw_card(1, target_card);
            }
        });
    }
}
#include "bounty.h"

#include "game/game.h"

namespace banggame {
    
    void effect_bounty::on_enable(card *target_card, player *p) {
        p->m_game->add_listener<event_type::before_hit>(target_card, [=](card *origin_card, player *origin, player *target, int damage, effect_flags flags) {
            if (origin && target == p && bool(flags & effect_flags::is_bang)) {
                origin->m_game->flash_card(target_card);
                origin->draw_card(1, target_card);
            }
        });
    }
}
#include "bounty.h"

#include "../../game.h"

namespace banggame {
    
    void effect_bounty::on_enable(card *target_card, player *p) {
        p->m_game->add_listener<event_type::before_hit>(target_card, [=](card *origin_card, player *origin, player *target, int damage, bool is_bang) {
            if (origin && target == p && is_bang) {
                origin->m_game->flash_card(target_card);
                origin->draw_card(1, target_card);
            }
        });
    }
}
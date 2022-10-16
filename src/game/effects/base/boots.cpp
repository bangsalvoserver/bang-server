#include "boots.h"

#include "../../game.h"

namespace banggame {
    
    void effect_boots::on_enable(card *target_card, player *p) {
        p->m_game->add_listener<event_type::after_hit>({target_card, 1}, [p, target_card](card *origin_card, player *origin, player *target, int damage, bool is_bang) {
            if (p == target) {
                target->m_game->queue_action([=]{
                    if (target->alive()) {
                        target->m_game->flash_card(target_card);
                        target->draw_card(damage, target_card);
                    }
                });
            }
        });
    }
}
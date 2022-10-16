#include "luckycharm.h"

#include "../../game.h"

namespace banggame {

    void effect_luckycharm::on_enable(card *target_card, player *p) {
        p->m_game->add_listener<event_type::after_hit>(target_card, [=](card *origin_card, player *origin, player *target, int damage, bool is_bang) {
            if (p == target) {
                target->m_game->queue_action([=]{
                    if (target->alive()) {
                        target->m_game->flash_card(target_card);
                        target->add_gold(damage);
                    }
                });
            }
        });
    }
}
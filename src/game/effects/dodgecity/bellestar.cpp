#include "bellestar.h"

#include "../../game.h"

namespace banggame {

    void effect_bellestar::on_enable(card *target_card, player *p) {
        p->m_game->add_listener<event_type::on_turn_start>(target_card, [=](player *target) {
            if (p == target) {
                p->m_game->add_disabler(target_card, [=](card *c) {
                    return c->pocket == pocket_type::player_table && c->owner != target;
                });
            }
        });
        p->m_game->add_listener<event_type::on_turn_end>(target_card, [=](player *target, bool skipped) {
            if (p == target) {
                p->m_game->remove_disablers(target_card);
            }
        });
    }

    void effect_bellestar::on_disable(card *target_card, player *target) {
        target->m_game->remove_disablers(target_card);
        target->m_game->remove_listeners(target_card);
    }
}
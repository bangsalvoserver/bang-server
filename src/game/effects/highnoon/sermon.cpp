#include "sermon.h"

#include "../../game.h"

namespace banggame {

    void effect_sermon::on_enable(card *target_card, player *target) {
        target->m_game->add_listener<event_type::pre_turn_start>(target_card, [=](player *p) {
            target->m_game->add_disabler(target_card, [=](card *c) {
                if (c->owner != p) {
                    return false;
                } else if (c->pocket == pocket_type::player_hand) {
                    return p->is_bangcard(c);
                } else {
                    return c->has_tag(tag_type::play_as_bang);
                }
            });
        });
        target->m_game->add_listener<event_type::on_turn_end>(target_card, [=](player *p, bool skipped) {
            target->m_game->remove_disablers(target_card);
        });
    }

    void effect_sermon::on_disable(card *target_card, player *target) {
        target->m_game->remove_disablers(target_card);
        target->m_game->remove_listeners(target_card);
    }
}
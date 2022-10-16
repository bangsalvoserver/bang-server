#include "big_spencer.h"

#include "../../game.h"

namespace banggame {

    void effect_big_spencer::on_enable(card *target_card, player *p) {
        p->m_game->add_disabler(target_card, [=](card *c) {
            return c->pocket == pocket_type::player_hand
                && c->owner == p
                && c->has_tag(tag_type::missedcard);
        });
    }

    void effect_big_spencer::on_disable(card *target_card, player *p) {
        p->m_game->remove_disablers(target_card);
    }
}
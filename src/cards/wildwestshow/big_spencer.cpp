#include "big_spencer.h"

#include "game/game.h"
#include "cards/filter_enums.h"

namespace banggame {

    void equip_big_spencer::on_enable(card *target_card, player *p) {
        p->m_game->add_disabler(target_card, [=](card *c) {
            return c->pocket == pocket_type::player_hand
                && c->owner == p
                && c->has_tag(tag_type::missedcard);
        });
    }

    void equip_big_spencer::on_disable(card *target_card, player *p) {
        p->m_game->remove_disablers(target_card);
    }
}
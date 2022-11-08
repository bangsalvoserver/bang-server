#include "reverend.h"

#include "game/game.h"

namespace banggame {

    void effect_reverend::on_enable(card *target_card, player *target) {
        target->m_game->add_disabler(target_card, [](card *c) {
            return c->pocket == pocket_type::player_hand
                && c->has_tag(tag_type::beer);
        });
    }

    void effect_reverend::on_disable(card *target_card, player *target) {
        target->m_game->remove_disablers(target_card);
    }
}
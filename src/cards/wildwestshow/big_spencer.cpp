#include "big_spencer.h"

#include "game/game.h"
#include "cards/filter_enums.h"

namespace banggame {

    void equip_big_spencer::on_enable(card *target_card, player *target) {
        target->m_game->add_disabler(target_card, [=](card *c) {
            return c->pocket == pocket_type::player_hand
                && c->owner == target
                && c->has_tag(tag_type::missedcard);
        });

        target->m_game->add_listener<event_type::apply_initial_cards_modifier>(target_card, [target](player *p, int &value) {
            if (p == target) {
                value = 5;
            }
        });
    }

    void equip_big_spencer::on_disable(card *target_card, player *target) {
        target->m_game->remove_disablers(target_card);
        target->m_game->remove_listeners(target_card);
    }
}
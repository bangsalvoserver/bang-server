#include "sermon.h"

#include "game/game.h"
#include "cards/filter_enums.h"

namespace banggame {

    void equip_sermon::on_enable(card_ptr target_card, player_ptr target) {
        target->m_game->add_listener<event_type::pre_turn_start>(target_card, [=](player_ptr p) {
            target->m_game->add_disabler(target_card, {[=](const_card_ptr c) {
                if (c->owner && c->owner != p) {
                    return false;
                } else if (c->pocket == pocket_type::player_hand) {
                    return c->has_tag(tag_type::bangcard);
                } else {
                    return c->has_tag(tag_type::play_as_bang);
                }
            }, true});
        });
        target->m_game->add_listener<event_type::on_turn_end>(target_card, [=](player_ptr p, bool skipped) {
            target->m_game->remove_disablers(target_card);
        });
    }

    void equip_sermon::on_disable(card_ptr target_card, player_ptr target) {
        target->m_game->remove_disablers(target_card);
        target->m_game->remove_listeners(target_card);
    }
}
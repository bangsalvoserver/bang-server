#include "bellestar.h"

#include "cards/game_events.h"

#include "game/game_table.h"

namespace banggame {

    void equip_bellestar::on_enable(card_ptr target_card, player_ptr p) {
        p->m_game->add_listener<event_type::on_turn_start>(target_card, [=](player_ptr target) {
            if (p == target) {
                p->m_game->add_disabler(target_card, [=](const_card_ptr c) {
                    return c->pocket == pocket_type::player_table && c->owner != target;
                });
            }
        });
        p->m_game->add_listener<event_type::on_turn_end>(target_card, [=](player_ptr target, bool skipped) {
            if (p == target) {
                p->m_game->remove_disablers(target_card);
            }
        });
    }

    void equip_bellestar::on_disable(card_ptr target_card, player_ptr target) {
        target->m_game->remove_disablers(target_card);
        target->m_game->remove_listeners(target_card);
    }
}
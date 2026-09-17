#include "bellestar.h"

#include "cards/game_events.h"

#include "game/game_table.h"

#include "utils/range_utils.h"

namespace banggame {

    void equip_bellestar::on_enable(card_ptr target_card, player_ptr p) {
        p->m_game->add_listener<event_type::on_turn_start>(target_card, [=](player_ptr target) {
            if (p == target) {
                auto is_disabled_by_bellestar = [=](const_card_ptr c) {
                    return c->pocket == pocket_type::player_table && c->owner != target;
                };
                // only counts as a real ability use when there was at least one opposing
                // table card (Barrel, Sombrero, ...) actually being disabled this turn
                if (rn::any_of(target->m_game->m_players, [&](player_ptr other) {
                    return rn::any_of(other->m_table, is_disabled_by_bellestar);
                })) {
                    target->m_game->call_event(event_type::on_special_ability_used{ target });
                }
                p->m_game->add_disabler(target_card, is_disabled_by_bellestar);
            }
        });
        p->m_game->add_listener<event_type::on_turn_end>(target_card, [=](player_ptr target, bool skipped) {
            if (p == target) {
                p->m_game->remove_disabler({ target_card, 0 });
            }
        });
    }

    void equip_bellestar::on_disable(card_ptr target_card, player_ptr target) {
        target->m_game->remove_disabler({ target_card, 0 });
        target->m_game->remove_listeners({ target_card, 0 });
    }
}
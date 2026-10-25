#include "anezka.h"

#include "cards/game_events.h"

#include "game/game_table.h"

namespace banggame {

    void equip_anezka::on_enable(card_ptr target_card, player_ptr target) {
        auto used_other = std::make_shared<bool>(false);

        target->m_game->add_listener<event_type::on_turn_start>(target_card, [target, used_other](player_ptr origin) {
            if (origin == target) *used_other = false;
        });

        target->m_game->add_listener<event_type::check_anezka_can_heal>(target_card, [target, used_other](player_ptr origin, player_ptr e_target) -> bool {
            if (origin != target || e_target == origin) return true;
            return !*used_other;
        });

        target->m_game->add_listener<event_type::mark_anezka_healed>(target_card, [target, used_other](player_ptr origin, player_ptr e_target) {
            if (origin == target && e_target != origin) {
                *used_other = true;
            }
        });
    }

    game_string effect_anezka_heal::get_error(card_ptr origin_card, player_ptr origin, player_ptr target) {
        if (!origin->m_game->call_event(event_type::check_anezka_can_heal{origin, target})) {
            return "ERROR_ANEZKA_ALREADY_HEALED";
        }
        return {};
    }

    void effect_anezka_heal::on_play(card_ptr origin_card, player_ptr origin, player_ptr target) {
        target->heal(origin_card, origin, 1);
        origin->m_game->call_event(event_type::mark_anezka_healed{origin, target});
    }

}

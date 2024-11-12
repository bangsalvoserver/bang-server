#include "ruleset.h"

#include "game/game.h"
#include "game/game_options.h"

#include "effects/highnoon/ruleset.h"
#include "effects/base/deathsave.h"

namespace banggame {

    void ruleset_fistfulofcards::on_apply(game *game) {
        if (!game->m_options.expansions.contains(GET_RULESET(highnoon))) {
            game->add_listener<event_type::on_turn_switch>({nullptr, 2}, [](player_ptr origin) {
                if (origin == origin->m_game->m_first_player && !origin->m_game->m_scenario_deck.empty()) {
                    origin->m_game->draw_scenario_card();
                }
            });
        }

        game->add_listener<event_type::on_player_eliminated>(nullptr, [first = true](player_ptr origin, player_ptr target) mutable {
            if (std::exchange(first, false)) {
                target->m_game->add_listener<event_type::get_first_dead_player>(nullptr, [=](player_ptr &result) {
                    result = target;
                });
            }
        });
    }
}
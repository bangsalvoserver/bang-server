#include "ruleset.h"

#include "game/game.h"

namespace banggame {

    void ruleset_fistfulofcards::on_apply(game *game) {
        if (!bool(game->m_options.expansions & expansion_type::highnoon)) {
            game->add_listener<event_type::on_turn_switch>({nullptr, 2}, [](player *origin) {
                if (origin == origin->m_game->m_first_player && !origin->m_game->m_scenario_deck.empty()) {
                    origin->m_game->draw_scenario_card();
                }
            });
        }
    }
}
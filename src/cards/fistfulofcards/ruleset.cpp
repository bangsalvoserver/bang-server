#include "ruleset.h"

#include "game/game.h"

namespace banggame {

    void ruleset_fistfulofcards::on_apply(game *game) {
        game->add_listener<event_type::on_turn_switch>({nullptr, 2}, [](player *origin) {
            if (origin == origin->m_game->m_first_player && !origin->m_game->m_scenario_deck.empty()) {
                if (bool(origin->m_game->m_scenario_deck.back()->expansion & expansion_type::fistfulofcards)) {
                    origin->m_game->queue_action([=]{
                        origin->m_game->draw_scenario_card();
                    });
                }
            }
        });
    }
}
#include "ruleset.h"

#include "game/game.h"

namespace banggame {

    void ruleset_wildwestshow::on_apply(game *game) {
        game->add_listener<event_type::on_game_setup>({nullptr, 5}, [](player *origin) {
            if (!origin->m_game->m_wws_scenario_deck.empty()) {
                origin->m_game->set_card_visibility(origin->m_game->m_wws_scenario_deck.back(), nullptr, card_visibility::shown);
            }
        });
    }
}
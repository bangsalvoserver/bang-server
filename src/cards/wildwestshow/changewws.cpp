#include "changewws.h"

#include "game/game.h"

namespace banggame {

    void effect_changewws::on_play(card *origin_card, player *origin) {
        if (origin->m_last_played_card == origin_card) return;

        auto &scenario_deck = origin->m_game->m_wws_scenario_deck;
        auto &scenario_cards = origin->m_game->m_wws_scenario_cards;
        if (scenario_deck.empty()) return;
        
        origin->m_game->queue_action([origin, &scenario_deck, &scenario_cards]{
            if (scenario_deck.size() > 1) {
                origin->m_game->set_card_visibility(*(scenario_deck.rbegin() + 1), nullptr, card_visibility::shown, true);
            }
            if (!scenario_cards.empty()) {
                scenario_cards.back()->on_disable(origin->m_game->m_wws_scenario_holder);
            }

            origin->m_game->add_log("LOG_DRAWN_SCENARIO_CARD", scenario_deck.back());
            if (origin->m_game->m_wws_scenario_holder != origin) {
                origin->m_game->m_wws_scenario_holder = origin;
                origin->m_game->add_update<game_update_type::move_scenario_deck>(origin->m_game->m_wws_scenario_holder, pocket_type::wws_scenario_deck);
            }
            origin->m_game->move_card(scenario_deck.back(), pocket_type::wws_scenario_card);
            scenario_cards.back()->on_enable(origin->m_game->m_wws_scenario_holder);
        }, 1);
    }
}
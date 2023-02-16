#include "changewws.h"

#include "game/game.h"

namespace banggame {

    void effect_changewws::on_play(card *origin_card, player *origin, const effect_context &ctx) {
        if (origin->m_game->m_wws_scenario_deck.empty()) return;
        if (ctx.repeating) return;
        
        origin->m_game->queue_action([origin]{
            auto &scenario_deck = origin->m_game->m_wws_scenario_deck;
            auto &scenario_cards = origin->m_game->m_wws_scenario_cards;
            
            if (scenario_deck.size() > 1) {
                origin->m_game->set_card_visibility(*(scenario_deck.rbegin() + 1), nullptr, card_visibility::shown, true);
            }
            if (!scenario_cards.empty()) {
                origin->m_game->m_wws_scenario_holder->disable_equip(scenario_cards.back());
            }

            origin->m_game->add_log("LOG_DRAWN_SCENARIO_CARD", scenario_deck.back());
            if (origin->m_game->m_wws_scenario_holder != origin) {
                origin->m_game->m_wws_scenario_holder = origin;
                origin->m_game->add_update<game_update_type::move_scenario_deck>(origin->m_game->m_wws_scenario_holder, pocket_type::wws_scenario_deck);
            }
            origin->m_game->move_card(scenario_deck.back(), pocket_type::wws_scenario_card);
            origin->m_game->m_wws_scenario_holder->enable_equip(scenario_cards.back());
        }, 1);
    }
}
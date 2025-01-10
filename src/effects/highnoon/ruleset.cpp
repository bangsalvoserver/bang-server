#include "ruleset.h"

#include "cards/game_events.h"

#include "game/game_table.h"

namespace banggame {
    
    void draw_scenario_card(game_ptr game) {
        if (!game->m_scenario_deck.empty() && game->m_scenario_deck.back()->visibility == card_visibility::hidden) {
            game->m_scenario_deck.back()->set_visibility(card_visibility::shown);
        } else {
            if (game->m_scenario_deck.size() > 1) {
                card_ptr second_card = *(game->m_scenario_deck.rbegin() + 1);
                second_card->set_visibility(card_visibility::shown, nullptr, true);
            }
            if (!game->m_scenario_cards.empty()) {
                game->m_first_player->disable_equip(game->m_scenario_cards.back());
            }
            game->add_log("LOG_DRAWN_SCENARIO_CARD", game->m_scenario_deck.back());
            game->m_scenario_deck.back()->move_to(pocket_type::scenario_card);
            game->m_first_player->enable_equip(game->m_scenario_cards.back());
        }
    }

    void ruleset_highnoon::on_apply(game_ptr game) {
        game->add_listener<event_type::on_turn_switch>({nullptr, 2}, [](player_ptr origin) {
            if (origin == origin->m_game->m_first_player && !origin->m_game->m_scenario_deck.empty()) {
                draw_scenario_card(origin->m_game);
            }
        });
    }
}
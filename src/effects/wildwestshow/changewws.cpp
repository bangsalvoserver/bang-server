#include "changewws.h"

#include "game/game_table.h"

namespace banggame {

    void effect_changewws::on_play(card_ptr origin_card, player_ptr origin, const effect_context &ctx) {
        auto &scenario_deck = origin->m_game->m_wws_scenario_deck;
        auto &scenario_cards = origin->m_game->m_wws_scenario_cards;

        if (scenario_deck.empty()) return;
        if (ctx.get<contexts::repeat_card>()) return;
        
        if (!scenario_cards.empty()) {
            origin->m_game->m_first_player->disable_equip(scenario_cards.back());
        }

        origin->m_game->add_log("LOG_DRAWN_SCENARIO_CARD", scenario_deck.back());
        scenario_deck.back()->move_to(pocket_type::wws_scenario_card);
        origin->m_game->m_first_player->enable_equip(scenario_cards.back());
    }
}
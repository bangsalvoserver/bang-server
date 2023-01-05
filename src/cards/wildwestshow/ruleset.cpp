#include "ruleset.h"

#include "game/game.h"

namespace banggame {

    void ruleset_wildwestshow::on_apply(game *game) {
        game->add_listener<event_type::on_effect_end>({nullptr, 1}, [](player *origin, card *origin_card) {
            if (origin_card->has_tag(tag_type::changewws) && origin->m_last_played_card != origin_card) {
                origin->m_game->queue_action([origin]{
                    auto &scenario_deck = origin->m_game->m_wws_scenario_deck;
                    auto &scenario_cards = origin->m_game->m_wws_scenario_cards;

                    if (scenario_deck.empty()) return;

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
        });
    }
}
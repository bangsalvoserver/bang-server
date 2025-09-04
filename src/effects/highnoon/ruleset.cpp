#include "ruleset.h"

#include "effects/base/pick.h"
#include "effects/base/death.h"
#include "effects/ghost_cards/ruleset.h"

#include "cards/game_events.h"
#include "cards/filter_enums.h"

#include "game/game_table.h"
#include "game/game_options.h"

namespace banggame {
    
    static void draw_scenario_card(game_ptr game) {
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

    struct request_choose_scenario : selection_picker {
        request_choose_scenario(player_ptr target)
            : selection_picker(nullptr, nullptr, target) {}

        void on_update() override {
            auto target_cards = target->m_game->m_scenario_deck
                | rv::take_while([](card_ptr target_card) {
                    return target_card->has_tag(tag_type::last_scenario_card);
                })
                | rn::to<std::vector>();

            if (target_cards.size() > 1) {
                for (card_ptr target_card : target_cards) {
                    target_card->move_to(pocket_type::selection);
                }
            } else {
                target->m_game->pop_request();
            }
        }

        void on_pick(card_ptr target_card) override {
            target->m_game->pop_request();

            target->m_game->add_log("LOG_CHOSE_CARD", target_card, target);

            target_card->move_to(pocket_type::scenario_deck, nullptr, card_visibility::shown, false, pocket_position::begin);
            target->m_game->remove_cards(target->m_game->m_selection);

            if (target->m_game->m_scenario_deck.size() > 1) {
                target_card->set_visibility(card_visibility::hidden, nullptr, true);
                target->m_game->add_short_pause();
            }
        }

        game_string status_text(player_ptr owner) const override {
            if (owner == target) {
                return "STATUS_CHOOSE_SCENARIO";
            } else {
                return {"STATUS_CHOOSE_SCENARIO_OTHER", target};
            }
        }
    };

    void ruleset_highnoon::on_apply(game_ptr game) {
        game->add_listener<event_type::on_game_setup>({nullptr, 5}, [](player_ptr origin) {
            origin->m_game->queue_request<request_choose_scenario>(origin);
        });

        game->add_listener<event_type::on_turn_switch>({nullptr, 2}, [first = true](player_ptr origin) mutable {
            if (origin == origin->m_game->m_first_player && !origin->m_game->m_scenario_deck.empty()) {
                if (std::exchange(first, false)) {
                    origin->m_game->m_scenario_deck.back()->set_visibility(card_visibility::shown);
                } else {
                    draw_scenario_card(origin->m_game);
                }
            }
        });
        
        if (game->m_options.expansions.contains(GET_RULESET(ghost_cards))) {
            game->add_listener<event_type::check_remove_player>(nullptr, [](bool &value) { value = false; });
        }
    }
}
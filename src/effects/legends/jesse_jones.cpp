#include "jesse_jones.h"

#include "effects/base/pick.h"

#include "cards/game_events.h"

#include "game/game_table.h"
#include "game/prompts.h"

namespace banggame {

    struct request_jesse_jones_legend : selection_picker {
        using selection_picker::selection_picker;

        void on_update() override {
            if (!live) {
                while (!origin->empty_hand()) {
                    card_ptr target_card = origin->m_hand.front();
                    target->m_game->add_log(update_target::includes(origin, target), "LOG_REVEALED_CARD", origin, target_card);
                    target_card->move_to(pocket_type::selection, target);
                }
            }
            auto_pick();
        }
        
        void on_pick(card_ptr target_card) override {
            target->m_game->pop_request();

            target->m_game->add_log(update_target::includes(origin, target), "LOG_STOLEN_CARD", target, origin, target_card);
            target->add_to_hand(target_card);

            while (!target->m_game->m_selection.empty()) {
                origin->add_to_hand(target->m_game->m_selection.front());
            }

            target->m_game->call_event(event_type::on_discard_hand_card{ origin, target_card, false });
        }

        game_string status_text(player_ptr owner) const override {
            if (owner == target) {
                return {"STATUS_JESSE_JONES_LEGEND", origin_card, origin};
            } else {
                return {"STATUS_JESSE_JONES_LEGEND_OTHER", origin_card, origin, target};
            }
        }
    };

    game_string effect_jesse_jones_legend::on_prompt(card_ptr origin_card, player_ptr origin, player_ptr target) {
        MAYBE_RETURN(prompts::bot_check_target_enemy(origin, target));
        return {};
    }

    void effect_jesse_jones_legend::on_play(card_ptr origin_card, player_ptr origin, player_ptr target) {
        origin->m_game->queue_request<request_jesse_jones_legend>(origin_card, target, origin);
    }
}
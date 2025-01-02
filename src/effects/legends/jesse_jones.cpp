#include "jesse_jones.h"

#include "effects/base/pick.h"

#include "game/game.h"
#include "game/prompts.h"

namespace banggame {

    struct request_jesse_jones_legend : selection_picker {
        using selection_picker::selection_picker;

        void on_update() override {
            if (!live) {
                while (!origin->empty_hand()) {
                    origin->m_hand.front()->move_to(pocket_type::selection, target);
                }
            }
            auto_pick();
        }
        
        void on_pick(card_ptr target_card) override {
            target->m_game->pop_request();

            target->add_to_hand(target_card);

            while (!target->m_game->m_selection.empty()) {
                origin->add_to_hand(target->m_game->m_selection.front());
            }
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
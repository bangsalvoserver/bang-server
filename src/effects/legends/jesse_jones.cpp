#include "jesse_jones.h"

#include "effects/base/pick.h"
#include "effects/base/steal_destroy.h"

#include "cards/game_events.h"
#include "cards/game_enums.h"

#include "game/game_table.h"
#include "game/prompts.h"

namespace banggame {

    struct request_jesse_jones_legend : request_picking {
        using request_picking::request_picking;

        void on_update() override {
            if (update_count == 0) {
                if (origin->m_hand.size() == 1) {
                    on_pick(origin->m_hand.front());
                } else {
                    origin->add_player_flags(player_flag::show_hand_playing);

                    for (card_ptr target_card : origin->m_hand) {
                        target->m_game->add_log(update_target::includes(origin, target), "LOG_REVEALED_CARD", origin, target_card);
                        target_card->set_visibility(update_target::includes(origin, target));
                    }
                }
            }
        }

        bool can_pick(const_card_ptr target_card) const override {
            return target_card->pocket == pocket_type::player_hand && target_card->owner == origin;
        }
        
        void on_pick(card_ptr target_card) override {
            target->m_game->pop_request();

            effect_steal{}.on_play(origin_card, target, target_card);

            target->m_game->queue_action([origin=origin]{
                for (card_ptr target_card : origin->m_hand) {
                    target_card->set_visibility(card_visibility::show_owner, origin);
                }
                
                origin->remove_player_flags(player_flag::show_hand_playing);
            }, 39);
        }

        game_string status_text(player_ptr owner) const override {
            if (owner == target) {
                return {"STATUS_JESSE_JONES_LEGEND", origin_card, origin};
            } else {
                return {"STATUS_JESSE_JONES_LEGEND_OTHER", origin_card, origin, target};
            }
        }
    };

    game_string effect_jesse_jones_legend::on_prompt(card_ptr origin_card, player_ptr origin, card_ptr target) {
        MAYBE_RETURN(prompts::bot_check_target_enemy(origin, target->owner));
        return {};
    }

    void effect_jesse_jones_legend::on_play(card_ptr origin_card, player_ptr origin, card_ptr target) {
        origin->m_game->queue_request<request_jesse_jones_legend>(origin_card, target->owner, origin);
    }
}
#include "falcon.h"

#include "cards/game_enums.h"

#include "effects/base/escapable.h"

#include "game/game_table.h"
#include "game/prompts.h"

namespace banggame {

    struct request_falcon : request_dismissable {
        using request_dismissable::request_dismissable;

        void on_update() override {
            if (!target->m_game->check_flags(game_flag::hands_shown)) {
                origin->add_player_flags(player_flag::show_hand_playing);

                for (card_ptr target_card : origin->m_hand) {
                    target->m_game->add_log(update_target::includes(origin, target), "LOG_REVEALED_CARD", origin, target_card);
                    target_card->set_visibility(player_set::includes(origin, target));
                }
            }
        }

        void on_resolve() override {
            pop_request();
    
            if (!target->m_game->check_flags(game_flag::hands_shown)) {
                for (card_ptr target_card : origin->m_hand) {
                    target_card->set_visibility(card_visibility::show_owner, origin);
                }
                
                origin->remove_player_flags(player_flag::show_hand_playing);
            }
        }

        game_string status_text(player_ptr owner) const override {
            if (owner == target) {
                return {"STATUS_FALCON", origin_card, origin};
            } else {
                return {"STATUS_FALCON_OTHER", origin_card, origin, target};
            }
        }
    };

    struct request_falcon_target : request_escapable {
        using request_escapable::request_escapable;

        void on_resolve() override {
            pop_request();
            target->m_game->queue_request<request_falcon>(origin_card, target, origin);
        }

        game_string status_text(player_ptr owner) const override {
            if (target == owner) {
                return {"STATUS_FALCON_TARGET", origin_card, origin};
            } else {
                return {"STATUS_FALCON_TARGET_OTHER", target, origin_card, origin};
            }
        }
    };

    prompt_string effect_falcon::on_prompt(card_ptr origin_card, player_ptr origin, player_ptr target, effect_flags flags) {
        MAYBE_RETURN(prompts::prompt_target_immunity(origin_card, origin, target, flags));
        if (target->empty_hand() || target->m_game->check_flags(game_flag::hands_shown)) {
            return {"PROMPT_CARD_NO_EFFECT", origin_card};
        }
        return {};
    }

    void effect_falcon::on_play(card_ptr origin_card, player_ptr origin, player_ptr target) {
        origin->m_game->queue_request<request_falcon_target>(origin_card, origin, target);
    }
}
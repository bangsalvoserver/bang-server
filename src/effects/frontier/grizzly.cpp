#include "grizzly.h"

#include "cards/game_enums.h"

#include "effects/base/escapable.h"

#include "game/game_table.h"
#include "game/prompts.h"

namespace banggame {

    struct request_grizzly : request_escapable {
        using request_escapable::request_escapable;

        void on_resolve() override {
            pop_request();
            target->damage(origin_card, origin, 1, flags);
        }

        game_string status_text(player_ptr owner) const override {
            if (target == owner) {
                return {"STATUS_GRIZZLY", origin_card};
            } else {
                return {"STATUS_GRIZZLY_OTHER", target, origin_card};
            }
        }
    };

    prompt_string effect_grizzly::on_prompt(card_ptr origin_card, player_ptr origin, player_ptr target, effect_flags flags) {
        MAYBE_RETURN(prompts::bot_check_target_enemy(origin, target, bot_suggestion::action_type::damage));
        MAYBE_RETURN(prompts::prompt_target_immunity(origin_card, origin, target, flags));
        return {};
    }

    void effect_grizzly::on_play(card_ptr origin_card, player_ptr origin, player_ptr target, effect_flags flags) {
        bot_suggestion::signal_hostile_action(origin, target, flags);

        origin->m_game->add_log("LOG_PLAYED_CARD_ON", origin_card, origin, target);
        origin->m_game->queue_request<request_grizzly>(origin_card, origin, target, flags);
    }
}
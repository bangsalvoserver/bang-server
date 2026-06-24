#include "chinatown.h"

#include "effects/base/draw.h"
#include "effects/base/requests.h"
#include "effects/base/escapable.h"

#include "game/game_table.h"
#include "game/prompts.h"
#include "game/bot_suggestion.h"
#include "game/game_options.h"

namespace banggame {

    struct request_chinatown : request_discard_hand, interface_escapable {
        using request_discard_hand::request_discard_hand;

        int ncards = 0;

        bool can_escape(card_ptr target_card) const override {
            return update_count == 0 && interface_escapable::can_escape(target_card);
        }

        void on_update() override {
            if (target->immune_to(origin_card, origin, flags)) {
                pop_request();
            } else if (update_count == 0) {
                ncards = target->m_hand.size();
            } else if (target->m_game->m_options.quick_discard_all || target->m_hand.size() <= 1) {
                on_resolve();
            }
        }

        void on_resolve() override {
            request_discard_hand::on_resolve();
            if (ncards > 0) {
                target->m_game->queue_request<request_can_draw>(origin_card, origin, target, ncards);
            }
        }
    };

    prompt_string effect_chinatown::on_prompt(card_ptr origin_card, player_ptr origin, player_ptr target, effect_flags flags) {
        MAYBE_RETURN(prompts::bot_check_target_enemy(origin, target));
        MAYBE_RETURN(prompts::prompt_target_immunity(origin_card, origin, target, flags));
        MAYBE_RETURN(prompts::prompt_target_self(origin_card, origin, target));

        if (target->empty_hand()) {
            return {"PROMPT_CARD_NO_EFFECT", origin_card};
        }
        return {};
    }

    void effect_chinatown::on_play(card_ptr origin_card, player_ptr origin, player_ptr target) {
        bot_suggestion::signal_hostile_action(origin, target);
        origin->m_game->queue_request<request_chinatown>(origin_card, origin, target);
    }
}
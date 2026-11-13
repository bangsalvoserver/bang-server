#include "lastwill.h"

#include "game/game_table.h"
#include "game/prompts.h"
#include "game/bot_suggestion.h"

#include "effects/base/death.h"
#include "effects/base/resolve.h"
#include "effects/base/gift_card.h"

namespace banggame {

    struct request_lastwill : request_dismissable {
        request_lastwill(card_ptr origin_card, player_ptr target)
            : request_dismissable(origin_card, nullptr, target) {}

        void on_update() override {
            if (target->m_hp > 0) {
                on_resolve();
            } else {
                auto_resolve();
            }
        }
        
        game_string status_text(player_ptr owner) const override {
            if (owner == target) {
                return {"STATUS_LASTWILL", origin_card};
            } else {
                return {"STATUS_LASTWILL_OTHER", origin_card, target};
            }
        }
    };

    game_string equip_lastwill::on_prompt(card_ptr origin_card, player_ptr origin, player_ptr target) const {
        MAYBE_RETURN(prompts::prompt_target_ghost(origin_card, origin, target));
        if (target->is_sheriff()) {
            return {"PROMPT_CARD_NO_EFFECT", origin_card};
        }
        return {};
    }

    int equip_lastwill::evaluate(card_ptr origin_card, player_ptr origin, player_ptr target) const {
        return bot_suggestion::evaluate_help(origin, target);
    }

    void equip_lastwill::on_enable(card_ptr origin_card, player_ptr origin) const {
        origin->m_game->add_listener<event_type::on_player_death>({origin_card, -1}, [=](player_ptr target, bool tried_save) {
            if (origin == target) {
                origin->m_game->queue_request<request_lastwill>(origin_card, origin);
            }
        });
    }

    bool effect_lastwill::can_play(card_ptr origin_card, player_ptr origin) const {
        return origin->m_game->top_request<request_lastwill>(target_is{origin}) != nullptr;
    }

    int effect_lastwill::evaluate(card_ptr origin_card, player_ptr origin, player_ptr target, const effect_context &ctx) const {
        return effect_gift_card{}.evaluate(origin_card, origin, target, ctx);
    }

    void effect_lastwill::on_play(card_ptr origin_card, player_ptr origin, player_ptr target, const effect_context &ctx) const {
        bot_suggestion::signal_helpful_action(origin, target);

        auto req = origin->m_game->top_request<request_lastwill>();
        req->pop_request();
        
        for (card_ptr chosen_card : ctx.get_all<contexts::gifted_card>()) {
            resolve_gift_card(origin_card, origin, chosen_card, target);
        }
    }
}

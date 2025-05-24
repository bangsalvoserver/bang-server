#include "ricochet.h"

#include "game/game_table.h"
#include "game/prompts.h"

#include "effects/base/bang.h"
#include "effects/base/steal_destroy.h"

namespace banggame {
    
    struct request_ricochet : request_targeting, missable_request {
        using request_targeting::request_targeting;

        void on_update() override {
            if (target->empty_hand()) {
                auto_resolve();
            }
        }

        void on_resolve() override {
            target->m_game->pop_request();
            effect_destroy{}.on_resolve(origin_card, origin, target_card);
        }

        void on_miss(card_ptr c, effect_flags missed_flags = {}) override {
            target->m_game->pop_request();
        }

        game_string status_text(player_ptr owner) const override {
            if (target == owner) {
                return {"STATUS_RICOCHET", origin_card, target_card};
            } else {
                return {"STATUS_RICOCHET_OTHER", target, origin_card, target_card};
            }
        }
    };

    prompt_string effect_ricochet::on_prompt(card_ptr origin_card, player_ptr origin, card_ptr target_card) {
        MAYBE_RETURN(prompts::bot_check_target_card(origin, target_card));
        return {};
    }

    void effect_ricochet::on_play(card_ptr origin_card, player_ptr origin, card_ptr target_card) {
        origin->m_game->queue_request<request_ricochet>(origin_card, origin, target_card->owner, target_card);
    }
}
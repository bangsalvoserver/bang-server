#include "frankie_canton.h"

#include "effects/base/resolve.h"

#include "game/game_table.h"
#include "game/game_options.h"
#include "game/prompts.h"

namespace banggame {

    game_string effect_frankie_canton::on_prompt(card_ptr origin_card, player_ptr origin, card_ptr target_card) {
        MAYBE_RETURN(prompts::bot_check_target_enemy(origin, target_card->owner));
        MAYBE_RETURN(prompts::prompt_target_self(origin_card, origin, target_card->owner));
        return {};
    }

    struct request_frankie_canton : request_resolvable_timer {
        request_frankie_canton(card_ptr origin_card, player_ptr origin, card_ptr target_card)
            : request_resolvable_timer{origin_card, origin, target_card->owner}
            , target_card{target_card} {}
        
        card_ptr target_card;

        card_list get_highlights(player_ptr owner) const override {
            return { target_card };
        }

        void on_update() override {
            origin->m_game->add_log("LOG_PLAYED_CARD_ON", origin_card, origin, target_card);
            set_duration(origin->m_game->m_options.auto_resolve_timer);
        }

        void on_resolve() override {
            origin->m_game->pop_request();
            target_card->move_cubes(origin->get_character(), 1);
        }

        game_string status_text(player_ptr owner) const override {
            if (owner == target) {
                return {"STATUS_FRANKIE_CANTON", origin_card, target_card};
            } else {
                return {"STATUS_FRANKIE_CANTON_OTHER", target, origin_card, target_card};
            }
        }
    };

    void effect_frankie_canton::on_play(card_ptr origin_card, player_ptr origin, card_ptr target_card) {
        origin->m_game->queue_request<request_frankie_canton>(origin_card, origin, target_card);
    }
}
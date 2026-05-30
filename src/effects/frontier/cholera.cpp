#include "cholera.h"

#include "effects/armedanddangerous/ruleset.h"
#include "effects/base/draw_check.h"

#include "cards/game_events.h"

#include "game/game_table.h"
#include "game/prompts.h"

namespace banggame {
    
    game_string equip_cholera::on_prompt(card_ptr origin_card, player_ptr origin, player_ptr target) {
        MAYBE_RETURN(prompts::bot_check_target_enemy(origin, target));
        MAYBE_RETURN(prompts::prompt_target_self(origin_card, origin, target));
        return {};
    }

    void equip_cholera::on_enable(card_ptr target_card, player_ptr target) {
        target->m_game->add_listener<event_type::on_turn_end>({ target_card, 25 }, [=](player_ptr origin, bool skipped) {
            if (origin == target) {
                target->m_game->queue_request<request_check>(target, target_card,
                    [=](card_sign sign) {
                        return draw_check_result {
                            .lucky = !sign.is_spades(),
                            .indifferent = target_card->num_cubes() == max_cubes_per_card
                        };
                    },
                    [=](bool result) {
                        if (!result) {
                            target_card->add_cubes(1);
                        }
                    }
                );
            }
        });
    }
}
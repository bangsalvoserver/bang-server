#include "snake.h"

#include "cards/game_enums.h"

#include "game/game_table.h"
#include "game/prompts.h"

#include "effects/base/predraw_check.h"

namespace banggame {

    game_string equip_snake::on_prompt(card_ptr origin_card, player_ptr origin, player_ptr target) {
        MAYBE_RETURN(prompts::bot_check_target_enemy(origin, target));
        MAYBE_RETURN(prompts::prompt_target_self(origin_card, origin, target));
        return {};
    }
    
    void equip_snake::on_enable(card_ptr target_card, player_ptr target) {
        target->m_game->add_listener<event_type::on_predraw_check>(target_card, [=](player_ptr p, card_ptr e_card) {
            if (p == target && e_card == target_card) {
                target->m_game->queue_request<request_check>(target, target_card, [](card_sign sign) {
                    return draw_check_result{
                        .lucky = !sign.is_spades(),
                        .defensive_redraw = true
                    };
                }, [=](bool result) {
                    if (!result) {
                        target->m_game->add_log("LOG_CARD_HAS_EFFECT", target_card);
                        target->m_game->play_sound(sound_id::snake);
                        target->damage(target_card, nullptr, 1);
                    }
                });
            }
        });
    }
}
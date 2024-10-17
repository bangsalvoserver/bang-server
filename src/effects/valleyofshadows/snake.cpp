#include "snake.h"

#include "game/game.h"

#include "effects/base/predraw_check.h"
#include "effects/base/prompts.h"

namespace banggame {

    game_string equip_snake::on_prompt(card_ptr origin_card, player_ptr origin, player_ptr target) {
        MAYBE_RETURN(bot_check_target_enemy(origin, target));
        MAYBE_RETURN(prompt_target_self(origin_card, origin, target));
        return {};
    }
    
    void equip_snake::on_enable(card_ptr target_card, player_ptr target) {
        target->m_game->add_listener<event_type::on_predraw_check>(target_card, [=](player_ptr p, card_ptr e_card) {
            if (p == target && e_card == target_card) {
                target->m_game->queue_request<request_check>(target, target_card, std::not_fn(&card_sign::is_spades), [=](bool result) {
                    if (!result) {
                        target->m_game->add_log("LOG_CARD_HAS_EFFECT", target_card);
                        target->m_game->play_sound("snake");
                        target->damage(target_card, nullptr, 1);
                    }
                });
            }
        });
    }
}
#include "jail.h"

#include "predraw_check.h"
#include "draw_check.h"

#include "game/game.h"
#include "game/prompts.h"

namespace banggame {

    game_string equip_jail::on_prompt(card_ptr origin_card, player_ptr origin, player_ptr target) {
        MAYBE_RETURN(prompts::bot_check_target_enemy(origin, target));
        MAYBE_RETURN(prompts::prompt_target_self(origin_card, origin, target));
        return {};
    }

    void equip_jail::on_enable(card_ptr target_card, player_ptr target) {
        target->m_game->add_listener<event_type::on_predraw_check>(target_card, [=](player_ptr p, card_ptr e_card) {
            if (p == target && e_card == target_card) {
                target->m_game->queue_request<request_check>(target, target_card, &card_sign::is_hearts, [=](bool result) {
                    target->discard_card(target_card);
                    if (result) {
                        target->m_game->add_log("LOG_JAIL_BREAK", target);
                    } else {
                        target->m_game->add_log("LOG_SKIP_TURN", target);
                        target->skip_turn();
                    }
                });
            }
        });
    }
}
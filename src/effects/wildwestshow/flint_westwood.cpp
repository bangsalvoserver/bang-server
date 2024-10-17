#include "flint_westwood.h"

#include "game/game.h"
#include "game/prompts.h"

namespace banggame {

    game_string handler_flint_westwood::on_prompt(card_ptr origin_card, player_ptr origin, card_ptr chosen_card, card_ptr target_card) {
        MAYBE_RETURN(prompts::bot_check_target_enemy(origin, target_card->owner));
        return {};
    }

    void handler_flint_westwood::on_play(card_ptr origin_card, player_ptr origin, card_ptr chosen_card, card_ptr target_card) {
        auto target = target_card->owner;

        for (int i=2; i && !target->empty_hand(); --i) {
            card_ptr stolen_card = target->random_hand_card();
            if (stolen_card->visibility != card_visibility::shown) {
                target->m_game->add_log(update_target::includes(origin, target), "LOG_STOLEN_CARD", origin, target, stolen_card);
                target->m_game->add_log(update_target::excludes(origin, target), "LOG_STOLEN_CARD_FROM_HAND", origin, target);
            } else {
                target->m_game->add_log("LOG_STOLEN_CARD", origin, target, stolen_card);
            }
            origin->steal_card(stolen_card);
        }
        if (chosen_card->visibility != card_visibility::shown) {
            target->m_game->add_log(update_target::includes(origin, target), "LOG_GIFTED_CARD", origin, target, chosen_card);
            target->m_game->add_log(update_target::excludes(origin, target), "LOG_GIFTED_A_CARD", origin, target);
        } else {
            target->m_game->add_log("LOG_GIFTED_CARD", origin, target, chosen_card);
        }
        target->steal_card(chosen_card);
    }
}
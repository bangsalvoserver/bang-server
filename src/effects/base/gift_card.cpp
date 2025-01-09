#include "gift_card.h"

#include "game/game_table.h"
#include "game/prompts.h"

#include "cards/game_enums.h"

namespace banggame {
    
    game_string handler_gift_card::on_prompt(card_ptr origin_card, player_ptr origin, card_ptr target_card, player_ptr target_player) {
        return prompts::bot_check_target_friend(origin, target_player);
    }

    void handler_gift_card::on_play(card_ptr origin_card, player_ptr origin, card_ptr target_card, player_ptr target_player) {
        if (!origin->m_game->check_flags(game_flag::hands_shown)) {
            origin->m_game->add_log(update_target::includes(origin, target_player), "LOG_GIFTED_CARD", origin, target_player, target_card);
            origin->m_game->add_log(update_target::excludes(origin, target_player), "LOG_GIFTED_A_CARD", origin, target_player);
        } else {
            origin->m_game->add_log("LOG_GIFTED_CARD", origin, target_player, target_card);
        }
        target_player->add_to_hand(target_card);
    }

    void handler_gift_card_selection::on_play(card_ptr origin_card, player_ptr origin, card_ptr target_card, player_ptr target_player) {
        handler_gift_card::on_play(origin_card, origin, target_card, target_player);

        while (!origin->m_game->m_selection.empty()) {
            card_ptr c = origin->m_game->m_selection.front();
            if (!origin->m_game->check_flags(game_flag::hands_shown)) {
                origin->m_game->add_log(update_target::includes(origin), "LOG_DRAWN_CARD", origin, c);
                origin->m_game->add_log(update_target::excludes(origin), "LOG_DRAWN_CARDS", origin, 1);
            } else {
                origin->m_game->add_log("LOG_DRAWN_CARD", origin, c);
            }
            origin->add_to_hand(c);
        }
    }
}
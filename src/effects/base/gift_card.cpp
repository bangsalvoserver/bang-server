#include "gift_card.h"

#include "game/game_table.h"
#include "game/prompts.h"

#include "cards/game_enums.h"
#include "cards/game_events.h"

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
        if (target_card->pocket == pocket_type::player_hand) {
            origin->m_game->call_event(event_type::on_discard_hand_card{ origin, target_card, used });
        }
        target_player->add_to_hand(target_card);
    }
}
#include "prompts.h"

#include "game_table.h"
#include "bot_suggestion.h"

#include "cards/filter_enums.h"

namespace banggame::prompts {

    game_string prompt_target_self(card_ptr origin_card, player_ptr origin, player_ptr target) {
        if (origin == target) {
            return {"PROMPT_TARGET_SELF", origin_card};
        }
        return {};
    }

    game_string prompt_target_ghost(card_ptr origin_card, player_ptr origin, player_ptr target) {
        if (target->is_ghost()) {
            return {"PROMPT_TARGET_GHOST", origin_card, target};
        } else {
            return {};
        }
    }

    prompt_string prompt_target_self_card(card_ptr origin_card, player_ptr origin, card_ptr target_card) {
        if (origin == target_card->owner) {
            if (target_card->pocket != pocket_type::player_table || !target_card->has_tag(tag_type::penalty)) {
                return {"PROMPT_TARGET_SELF", origin_card};
            }
        }
        return {};
    }

    prompt_string bot_check_kill_sheriff(player_ptr origin, player_ptr target) {
        if (origin->is_bot()
            && !(origin->m_role == player_role::outlaw || origin->m_role == player_role::renegade && origin->m_game->num_alive() <= 2)
            && (target->m_hp <= 1 && target->m_role == player_role::sheriff)
        ) {
            return {1, "BOT_DONT_KILL_SHERIFF"};
        }
        return {};
    }

    game_string bot_check_target_enemy(player_ptr origin, player_ptr target) {
        if (origin->is_bot() && !bot_suggestion::is_target_enemy(origin, target)) {
            return "BOT_TARGET_ENEMY";
        }
        return {};
    }

    game_string bot_check_target_friend(player_ptr origin, player_ptr target) {
        if (origin->is_bot() && !bot_suggestion::is_target_friend(origin, target)) {
            return "BOT_TARGET_FRIEND";
        }
        return {};
    }

    prompt_string bot_check_target_card(player_ptr origin, card_ptr target_card) {
        if (origin->is_bot()) {
            player_ptr target = target_card->owner;
            if (target->is_ghost()) {
                if (bot_suggestion::is_target_enemy(origin, target)) {
                    if (target_card->pocket == pocket_type::player_table && target_card->has_tag(tag_type::ghost_card)) {
                        return {};
                    } else {
                        return {1, "BOT_TARGET_NOT_GHOST_CARD"};
                    }
                } else if (bot_suggestion::is_target_friend(origin, target)) {
                    if (target_card->pocket == pocket_type::player_table && target_card->has_tag(tag_type::ghost_card)) {
                        return {1, "BOT_TARGET_GHOST_CARD"};
                    }
                }
                return "BOT_TARGET_ENEMY";
            }
            if (target_card->pocket == pocket_type::player_table && target_card->has_tag(tag_type::penalty)) {
                return bot_check_target_friend(origin, target);
            } else {
                return bot_check_target_enemy(origin, target);
            }
        }
        return {};
    }

    prompt_string bot_check_discard_card(player_ptr origin, card_ptr target) {
        if (origin->is_bot() && target->has_tag(tag_type::strong)) {
            return "BOT_TARGET_STRONG_CARD";
        }
        return {};
    }
}
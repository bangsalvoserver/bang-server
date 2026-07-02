#include "prompts.h"

#include "cards/filter_enums.h"
#include "cards/game_enums.h"

#include "effects/ghost_cards/ruleset.h"
#include "effects/frontier/ruleset.h"

#include "game_table.h"
#include "bot_suggestion.h"
#include "game_options.h"


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

    prompt_string prompt_target_immunity(card_ptr origin_card, player_ptr origin, player_ptr target, effect_flags flags) {
        if (origin->m_game->m_options.prompt_target_immunity || origin->is_bot()) {
            flags.add(effect_flag::is_prompt);
            if (target->immune_to(origin_card, origin, flags, true)) {
                return {"PROMPT_TARGET_IMMUNE", origin_card, target};
            }
        }
        return {};
    }

    prompt_string bot_check_kill_sheriff(player_ptr origin, player_ptr target) {
        if (origin->is_bot()) {
            switch (origin->get_base_role()) {
            case player_role::outlaw:
                break;
            case player_role::sheriff:
                if (bot_suggestion::is_target_friend(origin, target) && target->m_hp <= 1) {   
                    return {1, "BOT_DONT_KILL_DEPUTY"};
                }
                break;
            case player_role::renegade:
                if (origin->m_game->num_alive(true) <= 2) {
                    break;
                }
                [[fallthrough]];
            default:
                if (target->is_sheriff() && target->m_hp <= 1) {
                    return {1, "BOT_DONT_KILL_SHERIFF"};
                }
            }
        }
        return {};
    }

    game_string bot_check_target_enemy(player_ptr origin, player_ptr target, bool confident) {
        if (origin->is_bot() && !bot_suggestion::is_target_enemy(origin, target, confident)) {
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
                    if (target_card->pocket == pocket_type::player_table && is_ghost_card(target_card)) {
                        return {};
                    } else {
                        return {1, "BOT_TARGET_NOT_GHOST_CARD"};
                    }
                } else if (bot_suggestion::is_target_friend(origin, target)) {
                    if (target_card->pocket == pocket_type::player_table && is_ghost_card(target_card)) {
                        return {1, "BOT_TARGET_GHOST_CARD"};
                    }
                }
                return "BOT_TARGET_ENEMY";
            }
            if (target_card->pocket == pocket_type::player_table && target_card->is_purple()) {
                if (player_ptr tracked_player = get_tracked_player(target_card)) {
                    if (target_card->has_tag(tag_type::pardner_penalty)) {
                        return bot_check_target_friend(origin, tracked_player);
                    } else {
                        return bot_check_target_enemy(origin, tracked_player);
                    }
                }
            } else if (target_card->pocket == pocket_type::player_table && target_card->has_tag(tag_type::penalty)) {
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
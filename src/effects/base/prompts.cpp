#include "prompts.h"

#include "game/game.h"

namespace banggame {

    game_string prompt_target_self::on_prompt(card_ptr origin_card, player_ptr origin, player_ptr target) {
        if (origin == target) {
            return {"PROMPT_TARGET_SELF", origin_card};
        }
        return {};
    }

    game_string prompt_target_self::on_prompt(card_ptr origin_card, player_ptr origin, card_ptr target) {
        return on_prompt(origin_card, origin, target->owner);
    }

    game_string prompt_target_ghost::on_prompt(card_ptr origin_card, player_ptr origin, player_ptr target) {
        if (target->is_ghost()) {
            return {"PROMPT_TARGET_GHOST", origin_card, target};
        } else {
            return {};
        }
    }
}
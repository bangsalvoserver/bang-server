#ifndef __BASE_PROMPTS_H__
#define __BASE_PROMPTS_H__

#include "cards/card_effect.h"

namespace banggame::prompts {
    
    game_string prompt_target_self(card_ptr origin_card, player_ptr origin, player_ptr target);

    game_string prompt_target_ghost(card_ptr origin_card, player_ptr origin, player_ptr target);

    prompt_string prompt_target_self_card(card_ptr origin_card, player_ptr origin, card_ptr target_card);

    prompt_string bot_check_kill_sheriff(player_ptr origin, player_ptr target);

    game_string bot_check_target_enemy(player_ptr origin, player_ptr target);
    
    game_string bot_check_target_friend(player_ptr origin, player_ptr target);

    prompt_string bot_check_target_card(player_ptr origin, card_ptr target);

    prompt_string bot_check_discard_card(player_ptr origin, card_ptr target);
    
    template<typename R>
    concept prompt_range = rn::input_range<R> && std::convertible_to<rn::range_value_t<R>, prompt_string>;

    inline prompt_string select_prompt(prompt_range auto &&prompts) {
        prompt_string result;
        for (const prompt_string &str : prompts) {
            if (str && (!result || str.priority > result.priority)) {
                result = str;
            }
        }
        return result;
    }

    inline prompt_string select_prompt_fallback_empty(prompt_range auto &&prompts) {
        prompt_string result;
        bool found_empty = false;
        for (const prompt_string &str : prompts) {
            if (!str) {
                found_empty = true;
            } else if (!result || str.priority > result.priority) {
                result = str;
            }
        }
        if (found_empty && result.priority == 0) {
            result = {};
        }
        return result;
    }
}

#endif
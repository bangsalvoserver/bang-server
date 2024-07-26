#ifndef __BASE_PROMPTS_H__
#define __BASE_PROMPTS_H__

#include "cards/card_effect.h"

namespace banggame {
    
    struct prompt_target_self {
        game_string on_prompt(card_ptr origin_card, player_ptr origin, player_ptr target);
        game_string on_prompt(card_ptr origin_card, player_ptr origin, card_ptr target);
    };

    struct prompt_target_ghost {
        game_string on_prompt(card_ptr origin_card, player_ptr origin, player_ptr target);
    };
}

#endif
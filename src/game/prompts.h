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
}

#endif
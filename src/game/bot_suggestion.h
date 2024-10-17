#ifndef __BOT_SUGGESTION_H__
#define __BOT_SUGGESTION_H__

#include "cards/card_effect.h"

namespace banggame::bot_suggestion {

    bool is_target_enemy(player_ptr origin, player_ptr target);

    bool is_target_friend(player_ptr origin, player_ptr target);
    
}

#endif
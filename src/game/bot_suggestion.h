#ifndef __BOT_SUGGESTION_H__
#define __BOT_SUGGESTION_H__

#include "cards/card_effect.h"

namespace banggame::bot_suggestion {

    enum class action_type {
        neutral,
        damage,
        jail,
    };

    void signal_hostile_action(player_ptr origin, const_player_ptr target, effect_flags flags = {});

    void signal_helpful_action(player_ptr origin, const_player_ptr target, effect_flags flags = {});

    void signal_remove_card(player_ptr origin, const_card_ptr target_card, effect_flags flags = {});

    bool is_target_enemy(const_player_ptr origin, const_player_ptr target, action_type type = action_type::neutral);

    bool is_target_friend(const_player_ptr origin, const_player_ptr target);
    
}

#endif
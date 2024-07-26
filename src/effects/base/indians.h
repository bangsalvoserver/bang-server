#ifndef __BASE_INDIANS_H__
#define __BASE_INDIANS_H__

#include "cards/card_effect.h"
#include "game/bot_suggestion.h"
#include "prompts.h"

namespace banggame {
    
    struct effect_indians : prompt_target_ghost, bot_suggestion::target_enemy {
        void on_play(card_ptr origin_card, player_ptr origin, player_ptr target, effect_flags flags = {});
    };

    DEFINE_EFFECT(indians, effect_indians)
}

#endif
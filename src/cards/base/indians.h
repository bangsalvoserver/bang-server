#ifndef __BASE_INDIANS_H__
#define __BASE_INDIANS_H__

#include "cards/card_effect.h"
#include "game/bot_suggestion.h"
#include "prompts.h"

namespace banggame {
    
    struct effect_indians : prompt_target_ghost, bot_suggestion::target_enemy {
        void on_play(card *origin_card, player *origin, player *target, effect_flags flags = {});
    };
}

#endif
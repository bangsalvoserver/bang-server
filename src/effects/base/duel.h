#ifndef __BASE_DUEL_H__
#define __BASE_DUEL_H__

#include "cards/card_effect.h"
#include "game/bot_suggestion.h"
#include "effects/base/prompts.h"

namespace banggame {
    
    struct effect_duel : prompt_target_ghost, bot_suggestion::target_enemy {
        void on_play(card *origin_card, player *origin, player *target, effect_flags flags = {});
    };

    DEFINE_EFFECT(duel, effect_duel)
}

#endif
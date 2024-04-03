#ifndef __ARMEDANDDANGEROUS_FLINTLOCK_H__
#define __ARMEDANDDANGEROUS_FLINTLOCK_H__

#include "cards/card_effect.h"
#include "game/bot_suggestion.h"

namespace banggame {

    struct effect_flintlock : bot_suggestion::target_enemy {
        void on_play(card *origin_card, player *origin, player *target, const effect_context &ctx);
    };

    DEFINE_EFFECT(flintlock, effect_flintlock)
}

#endif
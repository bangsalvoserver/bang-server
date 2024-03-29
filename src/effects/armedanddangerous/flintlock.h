#ifndef __ARMEDANDDANGEROUS_FLINTLOCK_H__
#define __ARMEDANDDANGEROUS_FLINTLOCK_H__

#include "cards/card_effect.h"
#include "game/bot_suggestion.h"

namespace banggame {

    struct handler_flintlock {
        bool on_check_target(card *origin_card, player *origin, player *target, bool paid_cubes) {
            return bot_suggestion::target_enemy{}.on_check_target(origin_card, origin, target);
        }
        void on_play(card *origin_card, player *origin, player *target, bool paid_cubes);
    };

    DEFINE_MTH(flintlock, handler_flintlock)
}

#endif
#ifndef __ARMEDANDDANGEROUS_FRANKIE_CANTON_H__
#define __ARMEDANDDANGEROUS_FRANKIE_CANTON_H__

#include "cards/base/steal_destroy.h"

namespace banggame {

    struct effect_frankie_canton : prompt_target_self {
        game_string verify(card *origin_card, player *origin, card *target);
        void on_play(card *origin_card, player *origin, card *target);
    };
}

#endif
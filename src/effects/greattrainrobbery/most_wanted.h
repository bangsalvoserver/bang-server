#ifndef __GREATTRAINROBBERY_MOST_WANTED_H__
#define __GREATTRAINROBBERY_MOST_WANTED_H__

#include "cards/card_effect.h"

namespace banggame {

    struct effect_most_wanted {
        void on_play(card *origin_card, player *origin, player *target, effect_flags flags);
    };

    DEFINE_EFFECT(most_wanted, effect_most_wanted)
}

#endif
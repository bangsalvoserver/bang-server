#ifndef __GREATTRAINROBBERY_CIRCUS_WAGON_H__
#define __GREATTRAINROBBERY_CIRCUS_WAGON_H__

#include "cards/card_effect.h"

namespace banggame {

    struct effect_circus_wagon {
        void on_play(card *origin_card, player *origin, player *target);
    };

    DEFINE_EFFECT(circus_wagon, effect_circus_wagon)
}

#endif
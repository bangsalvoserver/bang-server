#ifndef __CANYONDIABLO_INDIANGUIDE_H__
#define __CANYONDIABLO_INDIANGUIDE_H__

#include "../card_effect.h"

namespace banggame {

    struct effect_indianguide : event_based_effect {
        void on_enable(card *target_card, player *target);
    };
}

#endif
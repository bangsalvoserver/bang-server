#ifndef __CANYONDIABLO_BROTHEL_H__
#define __CANYONDIABLO_BROTHEL_H__

#include "cards/card_effect.h"

namespace banggame {

    struct effect_brothel : event_based_effect {
        void on_enable(card *target_card, player *target);
    };
}

#endif
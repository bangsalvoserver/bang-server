#ifndef __WILDWESTSHOW_GARY_LOOTER__
#define __WILDWESTSHOW_GARY_LOOTER__

#include "cards/card_effect.h"

namespace banggame {

    struct effect_gary_looter : event_based_effect {
        void on_enable(card *target_card, player *target);
    };
}

#endif
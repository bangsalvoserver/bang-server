#ifndef __WILDWESTSHOW_JOHN_PAIN__
#define __WILDWESTSHOW_JOHN_PAIN__

#include "cards/card_effect.h"

namespace banggame {

    struct effect_john_pain : event_based_effect {
        void on_enable(card *target_card, player *target);
    };
}

#endif
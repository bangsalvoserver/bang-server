#ifndef __WILDWESTSHOW_TEREN_KILL__
#define __WILDWESTSHOW_TEREN_KILL__

#include "../card_effect.h"

namespace banggame {

    struct effect_teren_kill : event_based_effect {
        void on_enable(card *origin_card, player *origin);
    };
}

#endif
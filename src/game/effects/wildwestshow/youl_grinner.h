#ifndef __WILDWESTSHOW_YOUL_GRINNER__
#define __WILDWESTSHOW_YOUL_GRINNER__

#include "../card_effect.h"

namespace banggame {

    struct effect_youl_grinner : event_based_effect {
        void on_enable(card *target_card, player *target);
    };
}

#endif
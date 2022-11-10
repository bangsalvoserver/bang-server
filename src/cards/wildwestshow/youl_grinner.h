#ifndef __WILDWESTSHOW_YOUL_GRINNER__
#define __WILDWESTSHOW_YOUL_GRINNER__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_youl_grinner : event_equip {
        void on_enable(card *target_card, player *target);
    };
}

#endif
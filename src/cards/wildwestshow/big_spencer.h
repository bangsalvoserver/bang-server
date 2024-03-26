#ifndef __WILDWESTSHOW_BIG_SPENCER__
#define __WILDWESTSHOW_BIG_SPENCER__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_big_spencer {
        void on_enable(card *target_card, player *target);
        void on_disable(card *target_card, player *target);
    };

    DEFINE_EQUIP(big_spencer, equip_big_spencer)
}

#endif
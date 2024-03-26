#ifndef __WILDWESTSHOW_SHOWDOWN_H___
#define __WILDWESTSHOW_SHOWDOWN_H___

#include "cards/card_effect.h"

namespace banggame {

    struct equip_showdown {
        void on_enable(card *target_card, player *target);
        void on_disable(card *target_card, player *target);
    };

    DEFINE_EQUIP(showdown, equip_showdown)
}

#endif
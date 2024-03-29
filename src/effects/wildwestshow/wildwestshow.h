#ifndef __WILDWESTSHOW_WILDWESTSHOW_H___
#define __WILDWESTSHOW_WILDWESTSHOW_H___

#include "cards/card_effect.h"

namespace banggame {

    struct equip_wildwestshow {
        void on_enable(card *target_card, player *target);
        void on_disable(card *target_card, player *target);
    };

    DEFINE_EQUIP(wildwestshow, equip_wildwestshow)
}

#endif
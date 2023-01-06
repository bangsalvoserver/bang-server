#ifndef __WILDWESTSHOW_SACAGAWAY_H___
#define __WILDWESTSHOW_SACAGAWAY_H___

#include "cards/card_effect.h"

namespace banggame {

    struct equip_sacagaway {
        void on_enable(card *target_card, player *target);
        void on_disable(card *target_card, player *target);
    };
}

#endif
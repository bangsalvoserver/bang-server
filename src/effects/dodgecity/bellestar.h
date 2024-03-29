#ifndef __DODGECITY_BELLESTAR_H__
#define __DODGECITY_BELLESTAR_H__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_bellestar {
        void on_enable(card *target_card, player *target);
        void on_disable(card *target_card, player *target);
    };

    DEFINE_EQUIP(bellestar, equip_bellestar)
}

#endif
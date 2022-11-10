#ifndef __DODGECITY_GREG_DIGGER_H__
#define __DODGECITY_GREG_DIGGER_H__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_greg_digger : event_equip {
        void on_enable(card *target_card, player *target);
    };
}

#endif
#ifndef __HIGHNOON_SHOOTOUT_H__
#define __HIGHNOON_SHOOTOUT_H__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_shootout : event_equip {
        void on_enable(card *target_card, player *target);
    };

    DEFINE_EQUIP(shootout, equip_shootout)
}

#endif
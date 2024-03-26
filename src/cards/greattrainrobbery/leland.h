#ifndef __GREATTRAINROBBERY_LELAND_H__
#define __GREATTRAINROBBERY_LELAND_H__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_leland : event_equip {
        void on_enable(card *origin_card, player *origin);
    };

    DEFINE_EQUIP(leland, equip_leland)
}

#endif
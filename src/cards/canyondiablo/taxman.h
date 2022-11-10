#ifndef __CANYONDIABLO_TAXMAN_H__
#define __CANYONDIABLO_TAXMAN_H__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_taxman : event_equip {
        void on_enable(card *target_card, player *target);
    };
}

#endif
#ifndef __CANYONDIABLO_PAT_BARRETT_H__
#define __CANYONDIABLO_PAT_BARRETT_H__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_pat_barrett : event_equip {
        void on_enable(card *target_card, player *target);
    };
}

#endif
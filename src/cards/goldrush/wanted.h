#ifndef __GOLDRUSH_WANTED_H__
#define __GOLDRUSH_WANTED_H__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_wanted : event_equip {
        void on_enable(card *target_card, player *target);
    };
}

#endif
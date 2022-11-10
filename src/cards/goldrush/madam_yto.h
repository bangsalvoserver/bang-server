#ifndef __GOLDRUSH_MADAM_YTO_H__
#define __GOLDRUSH_MADAM_YTO_H__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_madam_yto : event_equip {
        void on_enable(card *target_card, player *origin);
    };
}

#endif
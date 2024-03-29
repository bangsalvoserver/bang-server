#ifndef __VALLEYOFSHADOWS_COLORADO_BILL_H__
#define __VALLEYOFSHADOWS_COLORADO_BILL_H__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_colorado_bill : event_equip {
        void on_enable(card *target_card, player *target);
    };

    DEFINE_EQUIP(colorado_bill, equip_colorado_bill)
}

#endif
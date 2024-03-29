#ifndef __DODGECITY_BILL_NOFACE_H__
#define __DODGECITY_BILL_NOFACE_H__

#include "cards/card_effect.h"

namespace banggame {
    
    struct equip_bill_noface : event_equip {
        void on_enable(card *target_card, player *target);
    };

    DEFINE_EQUIP(bill_noface, equip_bill_noface)
}

#endif
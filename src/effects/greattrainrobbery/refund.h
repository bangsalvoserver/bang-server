#ifndef __GREATTRAINROBBERY_REFUND_H__
#define __GREATTRAINROBBERY_REFUND_H__

#include "cards/card_effect.h"

namespace banggame {
    
    struct equip_refund : event_equip {
        void on_enable(card *origin_card, player *origin);
    };

    DEFINE_EQUIP(refund, equip_refund)
}

#endif
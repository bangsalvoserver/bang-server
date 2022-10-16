#ifndef __DODGECITY_BILL_NOFACE_H__
#define __DODGECITY_BILL_NOFACE_H__

#include "../card_effect.h"

namespace banggame {
    
    struct effect_bill_noface : event_based_effect {
        void on_enable(card *target_card, player *target);
    };
}

#endif
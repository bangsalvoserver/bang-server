#ifndef __VALLEYOFSHADOWS_COLORADO_BILL_H__
#define __VALLEYOFSHADOWS_COLORADO_BILL_H__

#include "../card_effect.h"

namespace banggame {

    struct effect_colorado_bill : event_based_effect {
        void on_enable(card *target_card, player *target);
    };
}

#endif
#ifndef __DODGECITY_GREG_DIGGER_H__
#define __DODGECITY_GREG_DIGGER_H__

#include "../card_effect.h"

namespace banggame {

    struct effect_greg_digger : event_based_effect {
        void on_enable(card *target_card, player *target);
    };
}

#endif
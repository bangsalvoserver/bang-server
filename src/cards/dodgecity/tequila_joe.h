#ifndef __DODGECITY_TEQUILA_JOE_H__
#define __DODGECITY_TEQUILA_JOE_H__

#include "cards/card_effect.h"

namespace banggame {

    struct effect_tequila_joe : event_based_effect {
        void on_enable(card *target_card, player *target);
    };
}

#endif
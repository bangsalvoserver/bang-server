#ifndef __ARMEDANDDANGEROUS_BLOODY_MARY_H__
#define __ARMEDANDDANGEROUS_BLOODY_MARY_H__

#include "../card_effect.h"

namespace banggame {

    struct effect_bloody_mary : event_based_effect {
        void on_enable(card *target_card, player *target);
    };

}

#endif
#ifndef __ARMEDANDDANGEROUS_BLOODY_MARY_H__
#define __ARMEDANDDANGEROUS_BLOODY_MARY_H__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_bloody_mary : event_equip {
        void on_enable(card *target_card, player *target);
    };

}

#endif
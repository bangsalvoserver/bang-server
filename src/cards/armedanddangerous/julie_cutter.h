#ifndef __ARMEDANDDANGEROUS_JULIE_CUTTER_H__
#define __ARMEDANDDANGEROUS_JULIE_CUTTER_H__

#include "cards/card_effect.h"

namespace banggame {

    struct effect_julie_cutter : event_based_effect {
        void on_enable(card *target_card, player *target);
    };

}

#endif
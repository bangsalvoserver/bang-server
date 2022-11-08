#ifndef __BASE_LUCKY_DUKE_H__
#define __BASE_LUCKY_DUKE_H__

#include "cards/card_effect.h"

namespace banggame {

    struct effect_lucky_duke : event_based_effect {
        void on_enable(card *target_card, player *target);
    };

}

#endif
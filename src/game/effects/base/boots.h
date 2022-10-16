#ifndef __BASE_BOOTS_H__
#define __BASE_BOOTS_H__

#include "../card_effect.h"

namespace banggame {

    struct effect_boots : event_based_effect {
        void on_enable(card *target_card, player *target);
    };
}

#endif
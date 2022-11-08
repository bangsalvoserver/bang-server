#ifndef __HIGHNOON_CURSE_H__
#define __HIGHNOON_CURSE_H__

#include "cards/card_effect.h"

namespace banggame {

    struct effect_curse : event_based_effect {
        void on_enable(card *target_card, player *target);
    };
}

#endif
#ifndef __BASE_SUZY_LAFAYETTE_H__
#define __BASE_SUZY_LAFAYETTE_H__

#include "cards/card_effect.h"

namespace banggame {

    struct effect_suzy_lafayette : event_based_effect {
        void on_enable(card *target_card, player *target);
    };
}

#endif
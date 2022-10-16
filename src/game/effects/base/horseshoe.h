#ifndef __BASE_HORSESHOE_H__
#define __BASE_HORSESHOE_H__

#include "../card_effect.h"

namespace banggame {

    struct effect_horseshoe : event_based_effect {
        void on_enable(card *target_card, player *target);

        static void queue(card *target_card, player *target);
    };

}

#endif
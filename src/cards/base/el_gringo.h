#ifndef __BASE_EL_GRINGO_H__
#define __BASE_EL_GRINGO_H__

#include "cards/card_effect.h"

namespace banggame {

    struct effect_el_gringo : event_based_effect {
        void on_enable(card *target_card, player *target);
    };
}

#endif
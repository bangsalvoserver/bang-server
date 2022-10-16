#ifndef __CANYONDIABLO_PACKMULE_H__
#define __CANYONDIABLO_PACKMULE_H__

#include "../card_effect.h"

namespace banggame {

    struct effect_packmule : event_based_effect {
        void on_enable(card *target_card, player *target);
    };
}

#endif
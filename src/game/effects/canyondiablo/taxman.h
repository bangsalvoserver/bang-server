#ifndef __CANYONDIABLO_TAXMAN_H__
#define __CANYONDIABLO_TAXMAN_H__

#include "../card_effect.h"

namespace banggame {

    struct effect_taxman : event_based_effect {
        void on_enable(card *target_card, player *target);
    };
}

#endif
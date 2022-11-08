#ifndef __HIGHNOON_HANDCUFFS_H__
#define __HIGHNOON_HANDCUFFS_H__

#include "cards/card_effect.h"

namespace banggame {

    struct effect_handcuffs : event_based_effect {
        void on_enable(card *target_card, player *target);
    };
}

#endif
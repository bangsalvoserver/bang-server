#ifndef __GOLDRUSH_PICKAXE_H__
#define __GOLDRUSH_PICKAXE_H__

#include "../card_effect.h"

namespace banggame {

    struct effect_pickaxe : event_based_effect {
        void on_enable(card *target_card, player *target);
    };
}

#endif
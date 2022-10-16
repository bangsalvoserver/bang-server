#ifndef __DODGECITY_MOLLY_STARK_H__
#define __DODGECITY_MOLLY_STARK_H__

#include "../card_effect.h"

namespace banggame {

    struct effect_molly_stark : event_based_effect {
        void on_enable(card *target_card, player *target);
    };
}

#endif
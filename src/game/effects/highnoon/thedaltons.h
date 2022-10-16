#ifndef __HIGHNOON_THEDALTONS_H__
#define __HIGHNOON_THEDALTONS_H__

#include "../card_effect.h"

namespace banggame {

    struct effect_thedaltons  {
        void on_enable(card *target_card, player *target);
    };
}

#endif
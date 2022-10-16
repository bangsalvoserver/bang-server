#ifndef __HIGHNOON_REVEREND_H__
#define __HIGHNOON_REVEREND_H__

#include "../card_effect.h"

namespace banggame {

    struct effect_reverend {
        void on_enable(card *target_card, player *target);
        void on_disable(card *target_card, player *target);
    };
}

#endif
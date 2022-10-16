#ifndef __HIGHNOON_HANGOVER_H__
#define __HIGHNOON_HANGOVER_H__

#include "../card_effect.h"

namespace banggame {

    struct effect_hangover {
        void on_enable(card *target_card, player *target);
        void on_disable(card *target_card, player *target);
    };
}

#endif
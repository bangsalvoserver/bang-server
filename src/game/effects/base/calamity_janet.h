#ifndef __BASE_CALAMITY_JANET_H__
#define __BASE_CALAMITY_JANET_H__

#include "../card_effect.h"

namespace banggame {

    struct effect_calamity_janet {
        void on_enable(card *target_card, player *target);
        void on_disable(card *target_card, player *target);
    };
}

#endif
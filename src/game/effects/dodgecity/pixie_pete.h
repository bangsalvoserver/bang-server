#ifndef __GOLDRUSH_PIXIE_PETE_H__
#define __GOLDRUSH_PIXIE_PETE_H__

#include "../card_effect.h"

namespace banggame {

    struct effect_pixie_pete : event_based_effect {
        void on_enable(card *target_card, player *target);
    };
}

#endif
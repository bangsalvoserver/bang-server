#ifndef __GOLDRUSH_DON_BELL_H__
#define __GOLDRUSH_DON_BELL_H__

#include "../card_effect.h"

namespace banggame {

    struct effect_don_bell : event_based_effect {
        void on_enable(card *target_card, player *origin);
    };
}

#endif
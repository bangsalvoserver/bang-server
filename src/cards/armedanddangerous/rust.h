#ifndef __ARMEDANDDANGEROUS_RUST_H__
#define __ARMEDANDDANGEROUS_RUST_H__

#include "cards/card_effect.h"

namespace banggame {

    struct effect_rust {
        void on_play(card *origin_card, player *origin, player *target, effect_flags flags = {});
    };

    DEFINE_EFFECT(rust, effect_rust)
}

#endif
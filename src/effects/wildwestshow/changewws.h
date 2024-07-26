#ifndef __WILDWESTSHOW_CHANGEWWS_H__
#define __WILDWESTSHOW_CHANGEWWS_H__

#include "cards/card_effect.h"

namespace banggame {

    struct effect_changewws {
        void on_play(card_ptr origin_card, player_ptr origin, const effect_context &ctx);
    };

    DEFINE_EFFECT(changewws, effect_changewws)
}

#endif
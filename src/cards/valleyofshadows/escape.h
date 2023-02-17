#ifndef __VALLEYOFSHADOWS_ESCAPE_H__
#define __VALLEYOFSHADOWS_ESCAPE_H__

#include "cards/card_effect.h"

namespace banggame {

    struct effect_escape {
        bool can_play(card *origin_card, player *origin);
        void on_play(card *origin_card, player *origin);
    };
}

#endif
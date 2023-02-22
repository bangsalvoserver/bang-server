#ifndef __ARMEDANDDANGEROUS_DOUBLEBARREL_H__
#define __ARMEDANDDANGEROUS_DOUBLEBARREL_H__

#include "cards/card_effect.h"
#include "cards/base/bang.h"

namespace banggame {

    struct effect_doublebarrel {
        void on_play(card *origin_card, player *origin);
    };

    struct modifier_doublebarrel : modifier_bangmod {
        game_string on_prompt(card *origin_card, player *origin, card *playing_card);
    };
}

#endif
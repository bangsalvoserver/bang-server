#ifndef __ARMEDANDDANGEROUS_DOUBLEBARREL_H__
#define __ARMEDANDDANGEROUS_DOUBLEBARREL_H__

#include "cards/card_effect.h"

namespace banggame {

    struct effect_doublebarrel {
        void on_play(card *origin_card, player *origin);
    };

    struct modifier_doublebarrel {
        game_string on_prompt(card *origin_card, player *origin, card *playing_card);
    };
}

#endif
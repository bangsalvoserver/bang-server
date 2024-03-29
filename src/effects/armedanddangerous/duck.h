#ifndef __ARMEDANDDANGEROUS_DUCK_H__
#define __ARMEDANDDANGEROUS_DUCK_H__

#include "cards/card_effect.h"

namespace banggame {

    struct handler_duck {
        game_string on_prompt(card *origin_card, player *origin, bool paid_cubes);
        void on_play(card *origin_card, player *origin, bool paid_cubes);
    };

    DEFINE_MTH(duck, handler_duck)
}

#endif
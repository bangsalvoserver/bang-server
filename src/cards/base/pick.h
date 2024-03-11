#ifndef __BASE_PICK_H__
#define __BASE_PICK_H__

#include "cards/card_effect.h"

namespace banggame {

    struct effect_pick {
        game_string on_prompt(card *origin_card, player *origin, card *target);
        void on_play(card *origin_card, player *origin, card *target);
    };

}

#endif
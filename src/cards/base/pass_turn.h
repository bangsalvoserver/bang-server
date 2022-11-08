#ifndef __BASE_PASS_TURN_H__
#define __BASE_PASS_TURN_H__

#include "cards/card_effect.h"

namespace banggame {

    struct effect_pass_turn {
        game_string verify(card *origin_card, player *origin);
        game_string on_prompt(card *origin_card, player *origin);
        void on_play(card *origin_card, player *origin);
    };
}

#endif
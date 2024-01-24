#ifndef __CANYONDIABLO_BUFFALO_BELL_H__
#define __CANYONDIABLO_BUFFALO_BELL_H__

#include "cards/card_effect.h"

namespace banggame {
    
    struct effect_buffalo_bell {
        game_string get_error(card *origin_card, player *origin, card *target);
        game_string on_prompt(card *origin_card, player *origin, card *target);
        void on_play(card *origin_card, player *origin, card *target);
    };
}

#endif
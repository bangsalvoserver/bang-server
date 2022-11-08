#ifndef __VALLEYOFSHADOWS_POKER_H__
#define __VALLEYOFSHADOWS_POKER_H__

#include "cards/card_effect.h"

namespace banggame {
    
    struct effect_poker {
        game_string on_prompt(card *origin_card, player *origin);
        void on_play(card *origin_card, player *origin);
    };
}

#endif
#ifndef __VALLEYOFSHADOWS_FANNING_H__
#define __VALLEYOFSHADOWS_FANNING_H__

#include "cards/card_effect.h"

namespace banggame {
    
    struct effect_fanning {
        game_string verify(card *origin_card, player *origin);
    };

    struct handler_fanning {
        game_string verify(card *origin_card, player *origin, player *player1, player *player2);
        void on_play(card *origin_card, player *origin, player *player1, player *player2);
    };
}

#endif
#ifndef __CANYONDIABLO_CARD_SHARPER_H__
#define __CANYONDIABLO_CARD_SHARPER_H__

#include "../card_effect.h"

namespace banggame {
    
    struct handler_card_sharper {
        game_string verify(card *origin_card, player *origin, card *chosen_card, card *target_card);
        void on_play(card *origin_card, player *origin, card *chosen_card, card *target_card);
    };
}

#endif
#ifndef __BASE_MAX_USAGES_H__
#define __BASE_MAX_USAGES_H__

#include "../card_effect.h"

namespace banggame {

    struct effect_max_usages {
        int max_usages;
        
        game_string verify(card *origin_card, player *origin);
        void on_play(card *origin_card, player *origin);
    };
}

#endif
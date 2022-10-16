#ifndef __ARMEDANDDANGEROUS_THUNDERER_H__
#define __ARMEDANDDANGEROUS_THUNDERER_H__

#include "../card_effect.h"

namespace banggame {
    
    struct effect_thunderer {
        void on_play(card *origin_card, player *origin);
    };
}

#endif
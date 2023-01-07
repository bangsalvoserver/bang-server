#ifndef __ARMEDANDDANGEROUS_BELLTOWER_H__
#define __ARMEDANDDANGEROUS_BELLTOWER_H__

#include "cards/card_effect.h"

namespace banggame {

    struct effect_belltower {
        verify_result verify(card *origin_card, player *origin);
    };
}

#endif
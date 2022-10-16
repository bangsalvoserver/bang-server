#ifndef __BASE_INDIANS_H__
#define __BASE_INDIANS_H__

#include "../card_effect.h"

namespace banggame {
    
    struct effect_indians {
        void on_play(card *origin_card, player *origin, player *target, effect_flags flags = {});
    };
}

#endif
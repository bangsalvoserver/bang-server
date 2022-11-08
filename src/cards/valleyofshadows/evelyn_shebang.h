#ifndef __VALLEYOFSHADOWS_EVELYN_SHEBANG_H__
#define __VALLEYOFSHADOWS_EVELYN_SHEBANG_H__

#include "cards/card_effect.h"

namespace banggame {
    
    struct effect_evelyn_shebang : event_based_effect {
        game_string verify(card *origin_card, player *origin, player *target);
        void on_play(card *origin_card, player *origin, player *target);
    };
}

#endif
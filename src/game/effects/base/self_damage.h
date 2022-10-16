#ifndef __BASE_SELF_DAMAGE_H__
#define __BASE_SELF_DAMAGE_H__

#include "../card_effect.h"

namespace banggame {
    
    struct effect_self_damage {
        game_string verify(card *origin_card, player *origin);
        void on_play(card *origin_card, player *origin);
    };
}

#endif
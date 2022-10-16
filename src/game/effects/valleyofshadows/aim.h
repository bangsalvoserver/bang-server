#ifndef __VALLEYOFSHADOWS_AIM_H__
#define __VALLEYOFSHADOWS_AIM_H__

#include "../card_effect.h"

namespace banggame {

    struct effect_aim {
        game_string verify(card *origin_card, player *origin);
        void on_play(card *origin_card, player *origin);
    };
}

#endif
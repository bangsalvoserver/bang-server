#ifndef __ARMEDANDDANGEROUS_BANDOLIER_H__
#define __ARMEDANDDANGEROUS_BANDOLIER_H__

#include "../card_effect.h"

namespace banggame {

    struct effect_bandolier : effect_empty {
        game_string verify(card *origin_card, player *origin);
    };
}

#endif
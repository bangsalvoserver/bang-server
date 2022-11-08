#ifndef __ARMEDANDDANGEROUS_BANDOLIER_H__
#define __ARMEDANDDANGEROUS_BANDOLIER_H__

#include "cards/card_effect.h"

namespace banggame {

    struct effect_bandolier {
        void on_play(card *origin_card, player *origin);
    };
}

#endif
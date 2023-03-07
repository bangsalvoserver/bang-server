#ifndef __GREATTRAINROBBERY_EXPRESS_CAR_H__
#define __GREATTRAINROBBERY_EXPRESS_CAR_H__

#include "cards/card_effect.h"

namespace banggame {

    struct effect_express_car {
        void on_play(card *origin_card, player *origin);
    };
}

#endif
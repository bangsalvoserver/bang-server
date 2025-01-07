#ifndef __GREATTRAINROBBERY_EXPRESS_CAR_H__
#define __GREATTRAINROBBERY_EXPRESS_CAR_H__

#include "cards/card_effect.h"

namespace banggame {

    struct effect_express_car {
        game_string get_error(card_ptr origin_card, player_ptr origin);
        void on_play(card_ptr origin_card, player_ptr origin);
    };

    DEFINE_EFFECT(express_car, effect_express_car)
}

#endif
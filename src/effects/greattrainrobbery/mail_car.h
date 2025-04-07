#ifndef __GREATTRAINROBBERY_MAIL_CAR_H__
#define __GREATTRAINROBBERY_MAIL_CAR_H__

#include "cards/card_effect.h"

namespace banggame {

    struct effect_mail_car {
        void on_play(card_ptr origin_card, player_ptr origin);
    };

    DEFINE_EFFECT(mail_car, effect_mail_car)

    struct effect_mail_car_response {
        bool can_play(card_ptr origin_card, player_ptr origin);
    };

    DEFINE_EFFECT(mail_car_response, effect_mail_car_response)
}

#endif
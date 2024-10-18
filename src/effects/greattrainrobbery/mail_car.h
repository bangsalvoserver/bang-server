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

    struct handler_mail_car {
        game_string on_prompt(card_ptr origin_card, player_ptr origin, card_ptr target_card, player_ptr target);
        void on_play(card_ptr origin_card, player_ptr origin, card_ptr target_card, player_ptr target);  
    };

    DEFINE_MTH(mail_car, handler_mail_car)
}

#endif
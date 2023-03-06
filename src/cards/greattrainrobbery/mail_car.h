#ifndef __GREATTRAINROBBERY_MAIL_CAR_H__
#define __GREATTRAINROBBERY_MAIL_CAR_H__

#include "cards/card_effect.h"

namespace banggame {

    struct effect_mail_car {
        void on_play(card *origin_card, player *origin);
    };

    struct handler_mail_car {
        game_string get_error(card *origin_card, player *origin, card *target_card, player *target);
        void on_play(card *origin_card, player *origin, card *target_card, player *target);  
    };
}

#endif
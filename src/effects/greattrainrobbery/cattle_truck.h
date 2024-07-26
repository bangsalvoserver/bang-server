#ifndef __GREATTRAINROBBERY_CATTLE_TRUCK_H__
#define __GREATTRAINROBBERY_CATTLE_TRUCK_H__

#include "cards/card_effect.h"

namespace banggame {

    struct effect_cattle_truck {
        game_string on_prompt(card_ptr origin_card, player_ptr origin);
        void on_play(card_ptr origin_card, player_ptr origin);
    };

    DEFINE_EFFECT(cattle_truck, effect_cattle_truck)
}

#endif
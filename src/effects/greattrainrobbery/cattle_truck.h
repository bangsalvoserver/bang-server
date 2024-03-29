#ifndef __GREATTRAINROBBERY_CATTLE_TRUCK_H__
#define __GREATTRAINROBBERY_CATTLE_TRUCK_H__

#include "cards/card_effect.h"

namespace banggame {

    struct effect_cattle_truck {
        game_string on_prompt(card *origin_card, player *origin);
        void on_play(card *origin_card, player *origin);
    };

    DEFINE_EFFECT(cattle_truck, effect_cattle_truck)
}

#endif
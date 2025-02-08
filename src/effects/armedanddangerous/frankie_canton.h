#ifndef __ARMEDANDDANGEROUS_FRANKIE_CANTON_H__
#define __ARMEDANDDANGEROUS_FRANKIE_CANTON_H__

#include "effects/base/steal_destroy.h"

namespace banggame {

    struct effect_frankie_canton {
        game_string on_prompt(card_ptr origin_card, player_ptr origin, card_ptr target);
        void on_play(card_ptr origin_card, player_ptr origin, card_ptr target);
    };

    DEFINE_EFFECT(frankie_canton, effect_frankie_canton)
}

#endif
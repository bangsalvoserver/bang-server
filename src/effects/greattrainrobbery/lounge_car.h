#ifndef __GREATTRAINROBBERY_LOUNGE_CAR_H__
#define __GREATTRAINROBBERY_LOUNGE_CAR_H__

#include "cards/card_effect.h"

namespace banggame {

    struct effect_lounge_car {
        game_string on_prompt(card_ptr origin_card, player_ptr origin) const;
        void on_play(card_ptr origin_card, player_ptr origin) const;
    };

    DEFINE_EFFECT(lounge_car, effect_lounge_car)

    struct effect_lounge_car_response {
        bool can_play(card_ptr origin_card, player_ptr origin) const;
        
        int evaluate(card_ptr origin_card, player_ptr origin, player_ptr target, const effect_context &ctx) const;
        game_string get_error(card_ptr origin_card, player_ptr origin, player_ptr target, const effect_context &ctx) const;
        void on_play(card_ptr origin_card, player_ptr origin, player_ptr target, const effect_context &ctx) const;
    };

    DEFINE_EFFECT(lounge_car_response, effect_lounge_car_response)
}

#endif

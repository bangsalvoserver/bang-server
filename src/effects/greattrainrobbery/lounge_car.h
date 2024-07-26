#ifndef __GREATTRAINROBBERY_LOUNGE_CAR_H__
#define __GREATTRAINROBBERY_LOUNGE_CAR_H__

#include "cards/card_effect.h"

#include "game/bot_suggestion.h"

namespace banggame {

    struct effect_lounge_car {
        game_string on_prompt(card_ptr origin_card, player_ptr origin);
        void on_play(card_ptr origin_card, player_ptr origin);
    };

    DEFINE_EFFECT(lounge_car, effect_lounge_car)

    struct effect_lounge_car_response {
        bool can_play(card_ptr origin_card, player_ptr origin);
    };

    DEFINE_EFFECT(lounge_car_response, effect_lounge_car_response)

    struct handler_lounge_car {
        bool on_check_target(card_ptr origin_card, player_ptr origin, card_ptr target_card, player_ptr target) {
            return bot_suggestion::target_friend{}.on_check_target(origin_card, origin, target);
        }
        game_string get_error(card_ptr origin_card, player_ptr origin, card_ptr target_card, player_ptr target);
        void on_play(card_ptr origin_card, player_ptr origin, card_ptr target_card, player_ptr target);
    };

    DEFINE_MTH(lounge_car, handler_lounge_car)
}

#endif
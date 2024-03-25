#ifndef __GREATTRAINROBBERY_LOUNGE_CAR_H__
#define __GREATTRAINROBBERY_LOUNGE_CAR_H__

#include "cards/card_effect.h"

#include "game/bot_suggestion.h"

namespace banggame {

    struct effect_lounge_car {
        game_string on_prompt(card *origin_card, player *origin);
        void on_play(card *origin_card, player *origin);
    };

    DEFINE_EFFECT(lounge_car, effect_lounge_car)

    struct handler_lounge_car {
        bool on_check_target(card *origin_card, player *origin, card *target_card, player *target) {
            return bot_suggestion::target_friend{}.on_check_target(origin_card, origin, target);
        }
        game_string get_error(card *origin_card, player *origin, card *target_card, player *target);
        void on_play(card *origin_card, player *origin, card *target_card, player *target);
    };

    DEFINE_MTH(lounge_car, handler_lounge_car)
}

#endif
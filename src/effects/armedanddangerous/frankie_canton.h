#ifndef __ARMEDANDDANGEROUS_FRANKIE_CANTON_H__
#define __ARMEDANDDANGEROUS_FRANKIE_CANTON_H__

#include "effects/base/steal_destroy.h"
#include "game/bot_suggestion.h"

namespace banggame {

    struct effect_frankie_canton : prompt_target_self, bot_suggestion::target_enemy {
        game_string get_error(card *origin_card, player *origin, card *target);
        void on_play(card *origin_card, player *origin, card *target);
    };

    DEFINE_EFFECT(frankie_canton, effect_frankie_canton)
}

#endif
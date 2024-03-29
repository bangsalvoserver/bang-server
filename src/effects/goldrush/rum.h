#ifndef __GOLDRUSH_RUM_H__
#define __GOLDRUSH_RUM_H__

#include "cards/card_effect.h"

namespace banggame {

    struct effect_rum {
        bool on_check_target(card *origin_card, player *origin);
        game_string on_prompt(card *origin_card, player *origin);
        void on_play(card *origin_card, player *origin);
    };

    DEFINE_EFFECT(rum, effect_rum)
}

#endif
#ifndef __GOLDRUSH_RUM_H__
#define __GOLDRUSH_RUM_H__

#include "cards/card_effect.h"

namespace banggame {

    struct effect_rum {
        game_string on_prompt(card_ptr origin_card, player_ptr origin) const;
        void on_play(card_ptr origin_card, player_ptr origin) const;
    };

    DEFINE_EFFECT(rum, effect_rum)
}

#endif
#ifndef __GOLDRUSH_JOSH_MCCLOUD_H__
#define __GOLDRUSH_JOSH_MCCLOUD_H__

#include "cards/card_effect.h"

namespace banggame {

    struct effect_forced_play {
        bool can_play(card_ptr origin_card, player_ptr origin);
        void on_play(card_ptr origin_card, player_ptr origin);
    };

    DEFINE_EFFECT(forced_play, effect_forced_play)

    struct effect_josh_mccloud {
        void on_play(card_ptr origin_card, player_ptr origin);
    };

    DEFINE_EFFECT(josh_mccloud, effect_josh_mccloud)
}

#endif
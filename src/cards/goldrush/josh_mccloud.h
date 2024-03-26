#ifndef __GOLDRUSH_JOSH_MCCLOUD_H__
#define __GOLDRUSH_JOSH_MCCLOUD_H__

#include "cards/card_effect.h"

namespace banggame {

    struct effect_forced_play {
        bool can_play(card *origin_card, player *origin);
        void on_play(card *origin_card, player *origin);
    };

    DEFINE_EFFECT(forced_play, effect_forced_play)

    struct effect_forced_equip {
        game_string get_error(card *origin_card, player *origin, player *target);
        game_string on_prompt(card *origin_card, player *origin, player *target);
        void on_play(card *origin_card, player *origin, player *target);
    };

    DEFINE_EFFECT(forced_equip, effect_forced_equip)

    struct effect_josh_mccloud {
        void on_play(card *origin_card, player *origin);
    };

    DEFINE_EFFECT(josh_mccloud, effect_josh_mccloud)
}

#endif
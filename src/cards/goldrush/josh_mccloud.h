#ifndef __GOLDRUSH_JOSH_MCCLOUD_H__
#define __GOLDRUSH_JOSH_MCCLOUD_H__

#include "cards/card_effect.h"

namespace banggame {

    struct effect_forced_play {
        verify_result verify(card *origin_card, player *origin);
        void on_play(card *origin_card, player *origin);
    };

    struct effect_forced_equip {
        game_string verify(card *origin_card, player *origin, player *target);
        game_string on_prompt(card *origin_card, player *origin, player *target);
        void on_play(card *origin_card, player *origin, player *target);
    };

    struct effect_josh_mccloud {
        void on_play(card *origin_card, player *origin);
    };
}

#endif
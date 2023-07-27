#ifndef __ARMEDANDDANGEROUS_RED_RINGO_H__
#define __ARMEDANDDANGEROUS_RED_RINGO_H__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_red_ringo {
        void on_enable(card *target_card, player *target);
    };

    struct handler_red_ringo {
        game_string get_error(card *origin_card, player *origin, const effect_target_list &targets);
        void on_play(card *origin_card, player *origin, const effect_target_list &targets);
    };
}

#endif
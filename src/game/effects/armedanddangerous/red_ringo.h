#ifndef __ARMEDANDDANGEROUS_RED_RINGO_H__
#define __ARMEDANDDANGEROUS_RED_RINGO_H__

#include "../card_effect.h"

namespace banggame {

    struct effect_red_ringo : event_based_effect {
        void on_equip(card *target_card, player *target);
    };

    struct handler_red_ringo {
        game_string verify(card *origin_card, player *origin, const target_list &targets);
        void on_play(card *origin_card, player *origin, const target_list &targets);
    };
}

#endif
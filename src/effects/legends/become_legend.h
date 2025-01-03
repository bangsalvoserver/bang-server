#ifndef __LEGENDS_BECOME_LEGEND_H__
#define __LEGENDS_BECOME_LEGEND_H__

#include "cards/card_effect.h"

namespace banggame {

    struct effect_become_legend {
        bool can_play(card_ptr origin_card, player_ptr origin);
        void on_play(card_ptr origin_card, player_ptr origin);
    };

    DEFINE_EFFECT(become_legend, effect_become_legend)
}

#endif
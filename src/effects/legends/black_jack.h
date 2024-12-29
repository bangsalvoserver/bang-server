#ifndef __LEGENDS_BLACK_JACK_H__
#define __LEGENDS_BLACK_JACK_H__

#include "cards/card_effect.h"

namespace banggame {

    struct effect_black_jack_legend {
        static int get_card_rank_value(card_rank rank);

        void on_play(card_ptr origin_card, player_ptr origin);
    };

    DEFINE_EFFECT(black_jack_legend, effect_black_jack_legend)

}

#endif
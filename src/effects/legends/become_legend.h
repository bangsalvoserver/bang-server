#ifndef __LEGENDS_BECOME_LEGEND_H__
#define __LEGENDS_BECOME_LEGEND_H__

#include "cards/card_effect.h"

namespace banggame {

    struct effect_become_legend {
        bool can_play(card_ptr origin_card, player_ptr origin) const;
        void on_play(card_ptr origin_card, player_ptr origin) const;
    };

    DEFINE_EFFECT(become_legend, effect_become_legend)

    struct effect_drop_all_fame {
        bool can_play(card_ptr origin_card, player_ptr origin) const;
        void on_play(card_ptr origin_card, player_ptr origin) const;
    };

    DEFINE_EFFECT(drop_all_fame, effect_drop_all_fame)
}

#endif
#ifndef __LEGENDS_JESSE_JONES_H__
#define __LEGENDS_JESSE_JONES_H__

#include "cards/card_effect.h"

namespace banggame {

    struct effect_jesse_jones_legend {
        game_string on_prompt(card_ptr origin_card, player_ptr origin, player_ptr target);
        void on_play(card_ptr origin_card, player_ptr origin, player_ptr target);
    };

    DEFINE_EFFECT(jesse_jones_legend, effect_jesse_jones_legend)

}

#endif
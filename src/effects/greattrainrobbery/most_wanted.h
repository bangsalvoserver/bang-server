#ifndef __GREATTRAINROBBERY_MOST_WANTED_H__
#define __GREATTRAINROBBERY_MOST_WANTED_H__

#include "cards/card_effect.h"

namespace banggame {

    struct effect_most_wanted {
        void on_play(card_ptr origin_card, player_ptr origin, player_ptr target, effect_flags flags);
    };

    DEFINE_EFFECT(most_wanted, effect_most_wanted)
}

#endif
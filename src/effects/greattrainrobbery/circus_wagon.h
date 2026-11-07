#ifndef __GREATTRAINROBBERY_CIRCUS_WAGON_H__
#define __GREATTRAINROBBERY_CIRCUS_WAGON_H__

#include "cards/card_effect.h"

namespace banggame {

    struct effect_circus_wagon {
        int evaluate(card_ptr origin_card, player_ptr origin, player_ptr target) const;
        void on_play(card_ptr origin_card, player_ptr origin, player_ptr target) const;
    };

    DEFINE_EFFECT(circus_wagon, effect_circus_wagon)
}

#endif

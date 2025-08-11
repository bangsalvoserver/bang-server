#ifndef __FISTFULOFCARDS_RANCH_H__
#define __FISTFULOFCARDS_RANCH_H__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_ranch : event_equip {
        void on_enable(card_ptr target_card, player_ptr target);
    };

    DEFINE_EQUIP(ranch, equip_ranch)

    struct effect_ranch {
        bool can_play(card_ptr origin_card, player_ptr origin);
        void on_play(card_ptr origin_card, player_ptr origin, const effect_context &ctx);
    };

    DEFINE_EFFECT(ranch, effect_ranch)
}

#endif
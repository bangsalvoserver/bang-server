#ifndef __CANYONDIABLO_SPIKE_SPIEZEL_H__
#define __CANYONDIABLO_SPIKE_SPIEZEL_H__

#include "cards/card_effect.h"

namespace banggame {

    struct modifier_spike_spiezel {
        game_string get_error(card *origin_card, player *origin, card *target_card, const effect_context &ctx);
        void add_context(card *origin_card, player *origin, effect_context &ctx);
    };
    
    DEFINE_MODIFIER(spike_spiezel, modifier_spike_spiezel)
}

#endif
#ifndef __GREATTRAINROBBERY_EVADED_H__
#define __GREATTRAINROBBERY_EVADED_H__

#include "cards/card_effect.h"
#include "effects/base/missed.h"

namespace banggame {

    struct effect_evaded : effect_missed {
        void on_play(card_ptr origin_card, player_ptr origin);
    };
    
    DEFINE_EFFECT(evaded, effect_evaded)
}

#endif
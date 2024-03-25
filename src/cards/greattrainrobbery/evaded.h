#ifndef __GREATTRAINROBBERY_EVADED_H__
#define __GREATTRAINROBBERY_EVADED_H__

#include "cards/card_effect.h"
#include "cards/base/missed.h"

namespace banggame {

    struct effect_evaded : effect_missed {
        void on_play(card *origin_card, player *origin);
    };
    
    DEFINE_EFFECT(evaded, effect_evaded)
}

#endif
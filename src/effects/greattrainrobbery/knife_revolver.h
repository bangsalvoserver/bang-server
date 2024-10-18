#ifndef __GREATTRAINROBBERY_KNIFE_REVOLVER_H__
#define __GREATTRAINROBBERY_KNIFE_REVOLVER_H__

#include "cards/card_effect.h"

namespace banggame {

    struct effect_knife_revolver {
        void on_play(card_ptr origin_card, player_ptr origin);
    };

    DEFINE_EFFECT(knife_revolver, effect_knife_revolver)
}

#endif
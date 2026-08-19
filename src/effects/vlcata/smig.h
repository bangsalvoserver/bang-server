#ifndef __VLCATA_SMIG_H__
#define __VLCATA_SMIG_H__

#include "cards/card_effect.h"

namespace banggame {
    struct effect_smig_destroy {
        void on_play(card_ptr origin_card, player_ptr origin, card_ptr target_card);
    };
    DEFINE_EFFECT(smig_destroy, effect_smig_destroy)
}

#endif
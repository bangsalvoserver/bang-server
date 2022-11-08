#ifndef __HIGHNOON_THEDOCTOR_H__
#define __HIGHNOON_THEDOCTOR_H__

#include "cards/card_effect.h"

namespace banggame {
    
    struct effect_thedoctor  {
        void on_enable(card *target_card, player *target);
    };
}

#endif
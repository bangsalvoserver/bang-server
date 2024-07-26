#ifndef __HIGHNOON_THEDOCTOR_H__
#define __HIGHNOON_THEDOCTOR_H__

#include "cards/card_effect.h"

namespace banggame {
    
    struct equip_thedoctor  {
        void on_enable(card_ptr target_card, player_ptr target);
    };

    DEFINE_EQUIP(thedoctor, equip_thedoctor)
}

#endif
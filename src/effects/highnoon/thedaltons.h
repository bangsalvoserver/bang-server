#ifndef __HIGHNOON_THEDALTONS_H__
#define __HIGHNOON_THEDALTONS_H__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_thedaltons  {
        void on_enable(card_ptr target_card, player_ptr target);
    };

    DEFINE_EQUIP(thedaltons, equip_thedaltons)
}

#endif
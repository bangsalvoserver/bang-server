#ifndef __HIGHNOON_INVERT_ROTATION_H__
#define __HIGHNOON_INVERT_ROTATION_H__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_invert_rotation  {
        void on_enable(card_ptr target_card, player_ptr target);
        void on_disable(card_ptr target_card, player_ptr target);
    };

    DEFINE_EQUIP(invert_rotation, equip_invert_rotation)
}

#endif
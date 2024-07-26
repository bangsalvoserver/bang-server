#ifndef __VALLEYOFSHADOWS_LEMONADE_JIM_H__
#define __VALLEYOFSHADOWS_LEMONADE_JIM_H__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_lemonade_jim : event_equip {
        void on_enable(card_ptr target_card, player_ptr target);
    };

    DEFINE_EQUIP(lemonade_jim, equip_lemonade_jim)
}

#endif
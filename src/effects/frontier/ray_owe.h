#ifndef __FRONTIER_RAY_OWE_H__
#define __FRONTIER_RAY_OWE_H__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_ray_owe {
        void on_enable(card_ptr target_card, player_ptr target);
        void on_disable(card_ptr target_card, player_ptr target);
    };

    DEFINE_EQUIP(ray_owe, equip_ray_owe)
}

#endif
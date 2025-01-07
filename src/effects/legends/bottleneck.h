#ifndef __LEGENDS_BOTTLENECK_H__
#define __LEGENDS_BOTTLENECK_H__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_bottleneck : event_equip {
        void on_enable(card_ptr target_card, player_ptr target);
    };

    DEFINE_EQUIP(bottleneck, equip_bottleneck)
}

#endif
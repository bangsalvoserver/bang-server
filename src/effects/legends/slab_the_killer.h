#ifndef __LEGENDS_SLAB_THE_KILLER_H__
#define __LEGENDS_SLAB_THE_KILLER_H__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_slab_the_killer_legend : event_equip {
        void on_enable(card_ptr target_card, player_ptr target);
    };

    DEFINE_EQUIP(slab_the_killer_legend, equip_slab_the_killer_legend)
}

#endif
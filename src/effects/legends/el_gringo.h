#ifndef __LEGENDS_EL_GRINGO_H__
#define __LEGENDS_EL_GRINGO_H__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_el_gringo_legend : event_equip {
        void on_enable(card_ptr target_card, player_ptr target);
    };

    DEFINE_EQUIP(el_gringo_legend, equip_el_gringo_legend)

}

#endif
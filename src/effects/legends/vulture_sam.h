#ifndef __LEGENDS_VULTURE_SAM_H__
#define __LEGENDS_VULTURE_SAM_H__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_vulture_sam_legend : event_equip {
        void on_enable(card_ptr target_card, player_ptr target);
    };

    DEFINE_EQUIP(vulture_sam_legend, equip_vulture_sam_legend)

}

#endif
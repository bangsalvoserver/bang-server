#ifndef __LEGENDS_SCRUGS_BALLAD_H__
#define __LEGENDS_SCRUGS_BALLAD_H__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_scrugs_ballad : event_equip {
        void on_enable(card_ptr target_card, player_ptr target);
    };

    DEFINE_EQUIP(scrugs_ballad, equip_scrugs_ballad)
}

#endif
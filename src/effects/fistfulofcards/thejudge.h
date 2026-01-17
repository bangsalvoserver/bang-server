#ifndef __FISTFULOFCARDS_THEJUDGE_H__
#define __FISTFULOFCARDS_THEJUDGE_H__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_thejudge : event_equip {
        void on_enable(card_ptr target_card, player_ptr target);
    };

    DEFINE_EQUIP(thejudge, equip_thejudge)

};

#endif
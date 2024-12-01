#ifndef __WILDWESTSHOW_BIG_SPENCER__
#define __WILDWESTSHOW_BIG_SPENCER__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_big_spencer : event_equip {
        void on_enable(card_ptr target_card, player_ptr target);
    };

    DEFINE_EQUIP(big_spencer, equip_big_spencer)
}

#endif
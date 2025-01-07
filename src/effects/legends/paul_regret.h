#ifndef __LEGENDS_PAUL_REGRET_H__
#define __LEGENDS_PAUL_REGRET_H__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_paul_regret_legend : event_equip {
        void on_enable(card_ptr target_card, player_ptr target);
    };

    DEFINE_EQUIP(paul_regret_legend, equip_paul_regret_legend)
}

#endif
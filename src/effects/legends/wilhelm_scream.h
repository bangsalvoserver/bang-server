#ifndef __LEGENDS_WILHELM_SCREAM_H__
#define __LEGENDS_WILHELM_SCREAM_H__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_wilhelm_scream : event_equip {
        void on_enable(card_ptr target_card, player_ptr target);
    };

    DEFINE_EQUIP(wilhelm_scream, equip_wilhelm_scream)
}

#endif
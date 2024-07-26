#ifndef __FISTFULOFCARDS_VENDETTA_H__
#define __FISTFULOFCARDS_VENDETTA_H__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_vendetta : event_equip {
        void on_enable(card_ptr target_card, player_ptr target);
    };

    DEFINE_EQUIP(vendetta, equip_vendetta)
}

#endif
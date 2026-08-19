#ifndef __VLCATA_BEJCEK_H__
#define __VLCATA_BEJCEK_H__

#include "cards/card_effect.h"

namespace banggame {
    struct bejcek_passive {
        void on_enable(card_ptr target_card, player_ptr target);
    };
    DEFINE_EQUIP(bejcek_passive, bejcek_passive)
}

#endif
#ifndef __GOLDRUSH_DON_BELL_H__
#define __GOLDRUSH_DON_BELL_H__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_don_bell : event_equip {
        void on_enable(card_ptr target_card, player_ptr origin);
    };

    DEFINE_EQUIP(don_bell, equip_don_bell)
}

#endif
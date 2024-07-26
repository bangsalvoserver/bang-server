#ifndef __GOLDRUSH_MADAM_YTO_H__
#define __GOLDRUSH_MADAM_YTO_H__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_madam_yto : event_equip {
        void on_enable(card_ptr target_card, player_ptr origin);
    };

    DEFINE_EQUIP(madam_yto, equip_madam_yto)
}

#endif
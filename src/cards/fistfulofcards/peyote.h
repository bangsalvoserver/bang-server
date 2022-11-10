#ifndef __FISTFULOFCARDS_PEYOTE_H__
#define __FISTFULOFCARDS_PEYOTE_H__

#include "cards/card_effect.h"

namespace banggame {
    
    struct equip_peyote : event_equip {
        void on_enable(card *target_card, player *target);
    };
}

#endif
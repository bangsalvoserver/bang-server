#ifndef __FISTFULOFCARDS_PEYOTE_H__
#define __FISTFULOFCARDS_PEYOTE_H__

#include "cards/card_effect.h"

namespace banggame {
    
    struct equip_peyote {
        void on_enable(card *target_card, player *target);
        void on_disable(card *target_card, player *target);
    };
}

#endif
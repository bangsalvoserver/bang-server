#ifndef __FISTFULOFCARDS_RUSSIANROULETTE_H__
#define __FISTFULOFCARDS_RUSSIANROULETTE_H__

#include "cards/card_effect.h"

namespace banggame {
    
    struct equip_russianroulette  {
        void on_enable(card *target_card, player *target);
    };

    DEFINE_EQUIP(russianroulette, equip_russianroulette)
}

#endif
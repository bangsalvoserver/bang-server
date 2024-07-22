#ifndef __FISTFULOFCARDS_PEYOTE_H__
#define __FISTFULOFCARDS_PEYOTE_H__

#include "cards/card_effect.h"

namespace banggame {
    
    struct equip_peyote {
        void on_enable(card *target_card, player *target);
        void on_disable(card *target_card, player *target);
    };

    DEFINE_EQUIP(peyote, equip_peyote)

    struct effect_peyote {
        int choice;
        effect_peyote(int choice): choice{choice} {}

        bool can_play(card *origin_card, player *origin);
        void on_play(card *origin_card, player *origin);
    };

    DEFINE_EFFECT(peyote, effect_peyote)
}

#endif
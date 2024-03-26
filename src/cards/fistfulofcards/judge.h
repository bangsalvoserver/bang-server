#ifndef __FISTFULOFCARDS_JUDGE_H__
#define __FISTFULOFCARDS_JUDGE_H__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_judge {
        void on_enable(card *target_card, player *target);
        void on_disable(card *target_card, player *target);
    };

    DEFINE_EQUIP(judge, equip_judge)
}

#endif
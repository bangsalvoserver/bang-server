#ifndef __FISTFULOFCARDS_LASSO_H__
#define __FISTFULOFCARDS_LASSO_H__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_lasso {
        void on_enable(card *target_card, player *target);
        void on_disable(card *target_card, player *target);
    };

    DEFINE_EQUIP(lasso, equip_lasso)
}

#endif
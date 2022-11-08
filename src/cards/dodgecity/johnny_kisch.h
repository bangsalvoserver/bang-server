#ifndef __DODGECITY_JOHHNY_KISCH_H__
#define __DODGECITY_JOHHNY_KISCH_H__

#include "cards/card_effect.h"

namespace banggame {

    struct effect_johnny_kisch : event_based_effect {
        void on_enable(card *target_card, player *target);
    };
}

#endif
#ifndef __DODGECITY_HERB_HUNTER_H__
#define __DODGECITY_HERB_HUNTER_H__

#include "../card_effect.h"

namespace banggame {

    struct effect_herb_hunter : event_based_effect {
        void on_enable(card *target_card, player *target);
    };
}

#endif
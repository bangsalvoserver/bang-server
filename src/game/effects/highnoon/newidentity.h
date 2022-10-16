#ifndef __HIGHNOON_NEWIDENTITY_H__
#define __HIGHNOON_NEWIDENTITY_H__

#include "../card_effect.h"

namespace banggame {

    struct effect_newidentity : event_based_effect {
        void on_enable(card *target_card, player *target);
    };

}

#endif
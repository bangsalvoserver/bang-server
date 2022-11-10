#ifndef __HIGHNOON_HANGOVER_H__
#define __HIGHNOON_HANGOVER_H__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_hangover {
        void on_enable(card *target_card, player *target);
        void on_disable(card *target_card, player *target);
    };
}

#endif
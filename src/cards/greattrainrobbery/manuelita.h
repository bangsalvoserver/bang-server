#ifndef __GREATTRAINROBBERY_MANUELITA_H__
#define __GREATTRAINROBBERY_MANUELITA_H__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_manuelita : event_equip {
        void on_enable(card *target_card, player *target);
    };
}

#endif
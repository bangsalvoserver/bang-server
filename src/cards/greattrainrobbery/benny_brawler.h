#ifndef __GREATTRAINROBBERY_BENNY_BRAWLER_H__
#define __GREATTRAINROBBERY_BENNY_BRAWLER_H__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_benny_brawler : event_equip {
        void on_enable(card *target_card, player *target);
    };
}

#endif
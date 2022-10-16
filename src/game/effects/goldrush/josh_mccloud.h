#ifndef __GOLDRUSH_JOSH_MCCLOUD_H__
#define __GOLDRUSH_JOSH_MCCLOUD_H__

#include "../card_effect.h"

namespace banggame {

    struct effect_josh_mccloud {
        void on_play(card *origin_card, player *origin);
    };
}

#endif
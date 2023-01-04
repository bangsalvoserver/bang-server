#ifndef __WILDWESTSHOW_LADYROSAOFTEXAS_H___
#define __WILDWESTSHOW_LADYROSAOFTEXAS_H___

#include "cards/card_effect.h"

namespace banggame {

    struct effect_ladyrosaoftexas {
        void on_play(card *origin_card, player *origin);
    };
}

#endif
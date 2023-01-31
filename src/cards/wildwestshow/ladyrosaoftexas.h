#ifndef __WILDWESTSHOW_LADYROSAOFTEXAS_H___
#define __WILDWESTSHOW_LADYROSAOFTEXAS_H___

#include "cards/card_effect.h"

namespace banggame {

    struct effect_ladyrosaoftexas {
        bool on_check_target(card *origin_card, player *origin);
        game_string verify(card *origin_card, player *origin);
        void on_play(card *origin_card, player *origin);
    };
}

#endif
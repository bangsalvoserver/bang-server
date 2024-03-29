#ifndef __WILDWESTSHOW_LADYROSAOFTEXAS_H___
#define __WILDWESTSHOW_LADYROSAOFTEXAS_H___

#include "cards/card_effect.h"

namespace banggame {

    struct effect_ladyrosaoftexas {
        bool on_check_target(card *origin_card, player *origin);
        game_string get_error(card *origin_card, player *origin);
        void on_play(card *origin_card, player *origin);
    };

    DEFINE_EFFECT(ladyrosaoftexas, effect_ladyrosaoftexas)
}

#endif
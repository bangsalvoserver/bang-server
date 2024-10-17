#ifndef __WILDWESTSHOW_LADYROSAOFTEXAS_H___
#define __WILDWESTSHOW_LADYROSAOFTEXAS_H___

#include "cards/card_effect.h"

namespace banggame {

    struct effect_ladyrosaoftexas {
        game_string get_error(card_ptr origin_card, player_ptr origin);
        game_string on_prompt(card_ptr origin_card, player_ptr origin);
        void on_play(card_ptr origin_card, player_ptr origin);
    };

    DEFINE_EFFECT(ladyrosaoftexas, effect_ladyrosaoftexas)
}

#endif
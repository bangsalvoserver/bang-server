#ifndef __GREATTRAINROBBERY_FULL_STEAM_H__
#define __GREATTRAINROBBERY_FULL_STEAM_H__

#include "cards/card_effect.h"

namespace banggame {

    struct effect_full_steam {
        int8_t value;
        effect_full_steam(int value) : value(value) {}
        
        void on_play(card_ptr origin_card, player_ptr origin);
    };

    DEFINE_EFFECT(full_steam, effect_full_steam)
}

#endif
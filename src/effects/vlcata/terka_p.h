#ifndef __VLCATA_TERKA_P_H__
#define __VLCATA_TERKA_P_H__

#include "cards/card_effect.h"

namespace banggame {
    struct equip_terka_p_bang : event_equip {
        void on_enable(card_ptr target_card, player_ptr target);
    };
    DEFINE_EQUIP(terka_p_bang, equip_terka_p_bang)

    struct effect_terka_p_selfheal {
        void on_play(card_ptr origin_card, player_ptr origin);
    };
    DEFINE_EFFECT(terka_p_selfheal, effect_terka_p_selfheal)
}

#endif
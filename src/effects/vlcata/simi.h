#ifndef __VLCATA_SIMI_H__
#define __VLCATA_SIMI_H__

#include "cards/card_effect.h"

namespace banggame {
    struct equip_simi_dynamite_master : event_equip {
        void on_enable(card_ptr target_card, player_ptr target);
    };
    DEFINE_EQUIP(simi_dynamite_master, equip_simi_dynamite_master)

    struct effect_simi_take_dynamite {
        void on_play(card_ptr origin_card, player_ptr origin);
    };
    DEFINE_EFFECT(simi_take_dynamite, effect_simi_take_dynamite)
}

#endif
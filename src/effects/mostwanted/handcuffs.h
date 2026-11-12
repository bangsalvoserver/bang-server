#ifndef __MOSTWANTED_HANDCUFFS_H__
#define __MOSTWANTED_HANDCUFFS_H__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_handcuffs : event_equip {
        void on_enable(card_ptr target_card, player_ptr target) const;
    };

    DEFINE_EQUIP(handcuffs, equip_handcuffs)

    struct effect_handcuffs {
        card_suit suit;
        effect_handcuffs(card_suit suit): suit{suit} {}

        bool can_play(card_ptr origin_card, player_ptr origin) const;
        void on_play(card_ptr origin_card, player_ptr origin) const;
    };

    DEFINE_EFFECT(handcuffs, effect_handcuffs)
}

#endif
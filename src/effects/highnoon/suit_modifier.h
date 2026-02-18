#ifndef __HIGHNOON_SUIT_MODIFIER_H__
#define __HIGHNOON_SUIT_MODIFIER_H__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_suit_modifier : event_equip {
        card_suit suit;
        equip_suit_modifier(card_suit suit): suit{suit} {}

        void on_enable(card_ptr target_card, player_ptr target);
    };

    DEFINE_EQUIP(suit_modifier, equip_suit_modifier)
}

#endif
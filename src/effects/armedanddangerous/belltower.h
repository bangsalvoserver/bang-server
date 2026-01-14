#ifndef __ARMEDANDDANGEROUS_BELLTOWER_H__
#define __ARMEDANDDANGEROUS_BELLTOWER_H__

#include "cards/card_effect.h"

namespace banggame {

    struct modifier_belltower {
        bool valid_with_equip(card_ptr origin_card, player_ptr origin, card_ptr playing_card) {
            return false;
        }
        bool valid_with_modifier(card_ptr origin_card, player_ptr origin, card_ptr playing_card) {
            return true;
        }
        bool valid_with_card(card_ptr origin_card, player_ptr origin, card_ptr playing_card);
        void add_context(card_ptr origin_card, player_ptr origin, effect_context &ctx);
    };
    
    DEFINE_MODIFIER(belltower, modifier_belltower)

    namespace contexts {
        struct ignore_distances {
            struct serialize_context{};
        };
    }
}

#endif
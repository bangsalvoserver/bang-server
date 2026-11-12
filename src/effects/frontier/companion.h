#ifndef __FRONTIER_COMPANION_H__
#define __FRONTIER_COMPANION_H__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_companion {
        void on_enable(card_ptr target_card, player_ptr target) const;
        void on_disable(card_ptr target_card, player_ptr target) const;
    };

    DEFINE_EQUIP(companion, equip_companion)

    struct modifier_companion {
        bool valid_with_equip(card_ptr origin_card, player_ptr origin, card_ptr playing_card) const {
            return false;
        }
        bool valid_with_modifier(card_ptr origin_card, player_ptr origin, card_ptr playing_card) const {
            return true;
        }
        bool valid_with_card(card_ptr origin_card, player_ptr origin, card_ptr playing_card) const;
        void add_context(card_ptr origin_card, player_ptr origin, effect_context &ctx) const;
    };

    DEFINE_MODIFIER(companion, modifier_companion)

    namespace contexts {
        struct distance_start {
            struct serialize_context{};
            player_ptr value;
        };
    }
}

#endif
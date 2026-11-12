#ifndef __BASE_GIFT_CARD_H__
#define __BASE_GIFT_CARD_H__

#include "cards/card_effect.h"

namespace banggame {

    struct effect_gift_card {
        bool used;
        
        effect_gift_card(bool used = false) : used{used} {}

        void add_context(card_ptr origin_card, player_ptr origin, effect_context &ctx) const {
            add_context(origin_card, origin, origin_card, ctx);
        }
        void add_context(card_ptr origin_card, player_ptr origin, card_ptr target_card, effect_context &ctx) const;

        int evaluate(card_ptr origin_card, player_ptr origin, player_ptr target, const effect_context &ctx) const;
        void on_play(card_ptr origin_card, player_ptr origin, player_ptr target, const effect_context &ctx) const;
    };

    DEFINE_EFFECT(gift_card, effect_gift_card)

    void resolve_gift_card(card_ptr origin_card, player_ptr origin, card_ptr target_card, player_ptr target, bool used = false);
    
    namespace contexts {
        struct gifted_card {
            card_ptr value;
        };
    }

}

#endif

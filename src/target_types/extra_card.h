#ifndef __TARGET_TYPE_EXTRA_CARD_H__
#define __TARGET_TYPE_EXTRA_CARD_H__

#include "cards/card_effect.h"

namespace banggame {

    struct targeting_extra_card : targeting_base {
        using targeting_base::targeting_base;

        using value_type = nullable_card;
        
        std::generator<card_ptr> possible_targets(const effect_context &ctx);
        card_ptr random_target(const effect_context &ctx);
        game_string get_error(const effect_context &ctx, card_ptr target);
        prompt_string on_prompt(const effect_context &ctx, card_ptr target);
        void add_context(effect_context &ctx, card_ptr target);
        void on_play(const effect_context &ctx, card_ptr target);
    };

    DEFINE_TARGETING(extra_card, targeting_extra_card)
}

#endif
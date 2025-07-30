#ifndef __TARGET_TYPE_CUBE_SLOT_H__
#define __TARGET_TYPE_CUBE_SLOT_H__

#include "cards/card_effect.h"

#include "card.h"

namespace banggame {

    struct targeting_cube_slot : targeting_card {
        using targeting_card::targeting_card;
        
        std::generator<card_ptr> possible_targets(const effect_context &ctx);
        card_ptr random_target(const effect_context &ctx);
        game_string get_error(const effect_context &ctx, card_ptr target);
    };

    DEFINE_TARGETING(cube_slot, targeting_cube_slot)
}

#endif
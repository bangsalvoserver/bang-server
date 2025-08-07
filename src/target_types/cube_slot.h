#ifndef __TARGET_TYPE_CUBE_SLOT_H__
#define __TARGET_TYPE_CUBE_SLOT_H__

#include "cards/card_effect.h"

#include "card.h"

namespace banggame {

    struct targeting_cube_slot : targeting_card {
        using targeting_card::targeting_card;
        
        auto possible_targets(const effect_context &ctx) {
            return origin->m_game->m_players
                | rv::for_each(&player::m_targetable_cards_view)
                | rv::filter([this, &ctx](card_ptr target) {
                    return !get_error(ctx, target);
                });
        }

        card_ptr random_target(const effect_context &ctx) {
            return random_element(possible_targets(ctx), origin->m_game->bot_rng);
        }

        game_string get_error(const effect_context &ctx, card_ptr target);
    };

    DEFINE_TARGETING(cube_slot, targeting_cube_slot)
}

#endif
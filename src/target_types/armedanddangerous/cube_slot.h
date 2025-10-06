#ifndef __TARGET_TYPE_CUBE_SLOT_H__
#define __TARGET_TYPE_CUBE_SLOT_H__

#include "cards/card_effect.h"

#include "target_types/base/card.h"

namespace banggame {

    struct targeting_cube_slot : targeting_card {
        bool stealing;

        targeting_cube_slot(target_args::player args, bool stealing = false)
            : targeting_card{{ .player_filter = args.player_filter }}
            , stealing{stealing} {}

        auto get_args() const {
            struct args {
                player_filter_bitset player_filter;
                bool stealing;
            };
            return args { player_filter, stealing };
        }
        
        auto possible_targets(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx) {
            return origin->m_game->m_players
                | rv::for_each([](player_ptr target) {
                    return rv::concat(
                        target->m_table,
                        target->m_characters | rv::take(1)
                    );
                })
                | rv::filter([=, &ctx, this](card_ptr target) {
                    return !get_error(origin_card, origin, effect, ctx, target);
                });
        }

        game_string get_error(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx, card_ptr target);
    };

    DEFINE_TARGETING(cube_slot, targeting_cube_slot)
}

#endif
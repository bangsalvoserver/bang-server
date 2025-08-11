#ifndef __TARGET_TYPE_CUBE_SLOT_H__
#define __TARGET_TYPE_CUBE_SLOT_H__

#include "cards/card_effect.h"

#include "target_types/base/card.h"

namespace banggame {

    struct targeting_cube_slot : targeting_card {
        bool stealing;

        targeting_cube_slot(targeting_args<bool, target_filter::card> args)
            : targeting_card{{ .player_filter = args.player_filter, .card_filter = args.card_filter }}
            , stealing{args.target_value} {}

        auto get_args() const {
            struct args {
                player_filter_bitset player_filter;
                card_filter_bitset card_filter;
                bool stealing;
            };
            return args { player_filter, card_filter, stealing };
        }
        
        auto possible_targets(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx) {
            return origin->m_game->m_players
                | rv::for_each(&player::m_targetable_cards_view)
                | rv::filter([=, &ctx, this](card_ptr target) {
                    return !get_error(origin_card, origin, effect, ctx, target);
                });
        }

        card_ptr random_target(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx) {
            return random_element(possible_targets(origin_card, origin, effect, ctx), origin->m_game->bot_rng);
        }

        game_string get_error(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx, card_ptr target);
    };

    DEFINE_TARGETING(cube_slot, targeting_cube_slot)
}

#endif
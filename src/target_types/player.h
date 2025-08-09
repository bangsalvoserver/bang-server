#ifndef __TARGET_TYPE_PLAYER_H__
#define __TARGET_TYPE_PLAYER_H__

#include "cards/card_effect.h"

#include "game/game_table.h"

namespace banggame {

    struct targeting_player {
        using value_type = player_ptr;

        player_filter_bitset player_filter;

        targeting_player(targeting_args<void, target_filter::player> args)
            : player_filter{args.player_filter} {}

        auto get_args() const {
            struct args {
                player_filter_bitset player_filter;
            };
            return args{ player_filter };
        }
        
        auto possible_targets(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx) {
            return origin->m_game->m_players
                | rv::filter([=, this](player_ptr target) {
                    return !get_error(origin_card, origin, effect, ctx, target);
                });
        }

        player_ptr random_target(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx) {
            return random_element(possible_targets(origin_card, origin, effect, ctx), origin->m_game->bot_rng);
        }

        game_string get_error(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx, player_ptr target);
        prompt_string on_prompt(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx, player_ptr target);
        void add_context(card_ptr origin_card, player_ptr origin, const effect_holder &effect, effect_context &ctx, player_ptr target);
        void on_play(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx, player_ptr target);
    };

    DEFINE_TARGETING(player, targeting_player)
}

#endif
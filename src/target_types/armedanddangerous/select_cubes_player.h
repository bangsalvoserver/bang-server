#ifndef __TARGET_TYPE_SELECT_CUBES_PLAYER_H__
#define __TARGET_TYPE_SELECT_CUBES_PLAYER_H__

#include "target_types/base/player.h"

#include "select_cubes_optional.h"

namespace banggame {

    struct targeting_select_cubes_player {
        struct value_type {
            struct transparent{};
            card_list cubes;
            player_ptr player;
        };

        targeting_select_cubes_optional target_cubes;
        targeting_player target_player;

        targeting_select_cubes_player(target_args::player args, int ncubes)
            : target_cubes{{}, ncubes}
            , target_player{args} {}
        
        auto get_args() const {
            struct args {
                player_filter_bitset player_filter;
                int ncubes;
            };
            return args{ target_player.player_filter, target_cubes.ncubes };
        }
        
        auto possible_targets(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx) {
            return target_player.possible_targets(origin_card, origin, effect, ctx)
                | rv::transform([](player_ptr target) {
                    return value_type{ .player = target };
                });
        }

        value_type random_target(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx) {
            auto cubes = target_cubes.random_target(origin_card, origin, effect, ctx);
            auto target = random_element(target_player.possible_targets(origin_card, origin, effect, ctx), origin->m_game->bot_rng);
            return { cubes, target };
        }

        game_string get_error(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx, const value_type &target) {
            MAYBE_RETURN(target_cubes.get_error(origin_card, origin, effect, ctx, target.cubes));
            MAYBE_RETURN(target_player.get_error(origin_card, origin, effect, ctx, target.player));
            return {};
        }

        prompt_string on_prompt(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx, const value_type &target) {
            return merge_prompts_strict(std::array{
                target_cubes.on_prompt(origin_card, origin, effect, ctx, target.cubes),
                target_player.on_prompt(origin_card, origin, effect, ctx, target.player)
            });
        }

        void add_context(card_ptr origin_card, player_ptr origin, const effect_holder &effect, effect_context &ctx, const value_type &target) {    
            target_cubes.add_context(origin_card, origin, effect, ctx, target.cubes);
            target_player.add_context(origin_card, origin, effect, ctx, target.player);
        }

        void on_play(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx, const value_type &target) {
            target_cubes.on_play(origin_card, origin, effect, ctx, target.cubes);
            target_player.on_play(origin_card, origin, effect, ctx, target.player);
        }
    };

    DEFINE_TARGETING(select_cubes_player, targeting_select_cubes_player)
}

#endif
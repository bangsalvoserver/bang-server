#ifndef __TARGET_TYPE_PLAYER_PER_CUBE_H__
#define __TARGET_TYPE_PLAYER_PER_CUBE_H__

#include "select_cubes.h"
#include "player.h"

namespace banggame {

    struct targeting_player_per_cube {
        struct value_type {
            struct transparent{};
            card_list cubes;
            player_list players;
        };

        targeting_select_cubes target_cubes;
        targeting_player target_player;
        int extra_players;

        targeting_player_per_cube(targeting_args<int, target_filter::player> args)
            : target_cubes{{}}
            , target_player{{ .player_filter = args.player_filter }}
            , extra_players{args.target_value} {}
        
        auto get_args() const {
            struct args {
                player_filter_bitset player_filter;
                int extra_players;
            };
            return args{ target_player.player_filter, extra_players };
        }
        
        bool is_possible(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx);
        value_type random_target(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx);
        game_string get_error(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx, const value_type &target);
        prompt_string on_prompt(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx, const value_type &target);
        void add_context(card_ptr origin_card, player_ptr origin, const effect_holder &effect, effect_context &ctx, const value_type &target);
        void on_play(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx, const value_type &target);
    };

    DEFINE_TARGETING(player_per_cube, targeting_player_per_cube)
}

#endif
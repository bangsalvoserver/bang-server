#ifndef __TARGET_TYPE_PLAYER_PER_CUBE_H__
#define __TARGET_TYPE_PLAYER_PER_CUBE_H__

#include "target_types/base/player.h"

#include "select_cubes.h"

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

        targeting_player_per_cube(target_args::player args, int extra_players)
            : target_cubes{{}, 0}
            , target_player{args}
            , extra_players{extra_players} {}
        
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
#ifndef __TARGET_TYPE_SELECT_CUBES_H__
#define __TARGET_TYPE_SELECT_CUBES_H__

#include "cards/card_effect.h"

namespace banggame {

    struct targeting_select_cubes {
        using value_type = card_list;

        int ncubes;

        targeting_select_cubes(targeting_args<int, target_filter::none> args)
            : ncubes{std::max(1, args.target_value)} {}
        
        auto get_args() const {
            struct args {
                int ncubes;
            };
            return args{ ncubes };
        }
        
        bool is_possible(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx);
        card_list random_target(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx);
        game_string get_error(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx, const card_list &target);
        prompt_string on_prompt(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx, const card_list &target);
        void add_context(card_ptr origin_card, player_ptr origin, const effect_holder &effect, effect_context &ctx, const card_list &target);
        void on_play(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx, const card_list &target);
    };

    DEFINE_TARGETING(select_cubes, targeting_select_cubes)
}

#endif
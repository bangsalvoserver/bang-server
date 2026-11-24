#ifndef __TARGET_TYPE_SELECT_CUBES_REPEAT_H__
#define __TARGET_TYPE_SELECT_CUBES_REPEAT_H__

#include "cards/card_effect.h"

#include "select_cubes.h"

namespace banggame {

    struct targeting_select_cubes_repeat : targeting_select_cubes {
        int max_repeats;

        targeting_select_cubes_repeat(target_args::empty args, int ncubes, int max_repeats = 0)
            : targeting_select_cubes{args, ncubes}
            , max_repeats{max_repeats} {}

        auto get_args() const {
            struct args {
                int ncubes;
                int max_repeats;
            };
            return args{ ncubes, max_repeats };
        }
        
        bool is_possible(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx) const;
        scored_target<card_list> random_target(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx) const;
        game_string get_error(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx, const card_list &target) const;
        prompt_string on_prompt(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx, const card_list &target) const;
        void add_context(card_ptr origin_card, player_ptr origin, const effect_holder &effect, effect_context &ctx, const card_list &target) const;
        void on_play(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx, const card_list &target) const;
    };

    DEFINE_TARGETING(select_cubes_repeat, targeting_select_cubes_repeat)
}

#endif

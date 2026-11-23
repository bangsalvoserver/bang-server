#ifndef __TARGET_TYPE_SELECT_CUBES_H__
#define __TARGET_TYPE_SELECT_CUBES_H__

#include "cards/card_effect.h"

#include "game/possible_to_play.h"

namespace banggame {

    using scored_cube = scored_target<card_ptr>;
    using scored_cube_list = std::vector<scored_cube>;

    scored_cube_list get_all_cubes(player_ptr origin, const effect_context &ctx);

    struct targeting_select_cubes_args {
        int ncubes;
    };

    struct targeting_select_cubes : targeting_select_cubes_args {
        using value_type = card_list;

        targeting_select_cubes(target_args::empty, int ncubes)
            : targeting_select_cubes_args{ncubes} {}
        
        const auto &get_args() const {
            return static_cast<const targeting_select_cubes_args &>(*this);
        }
        
        bool is_possible(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx) const;
        scored_target<card_list> random_target(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx) const;
        game_string get_error(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx, const card_list &target) const;
        prompt_string on_prompt(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx, const card_list &target) const;
        void add_context(card_ptr origin_card, player_ptr origin, const effect_holder &effect, effect_context &ctx, const card_list &target) const;
        void on_play(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx, const card_list &target) const;
    };

    DEFINE_TARGETING(select_cubes, targeting_select_cubes)
}

#endif
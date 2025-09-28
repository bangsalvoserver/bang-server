#ifndef __TARGET_TYPE_SELF_CUBES_H__
#define __TARGET_TYPE_SELF_CUBES_H__

#include "cards/card_effect.h"

namespace banggame {

    struct targeting_self_cubes_args {
        int ncubes;
    };

    struct targeting_self_cubes : targeting_self_cubes_args {
        using value_type = std::nullptr_t;
        
        targeting_self_cubes(target_args::empty, int ncubes)
            : targeting_self_cubes_args{ncubes} {}
        
        const auto &get_args() const {
            return static_cast<const targeting_self_cubes_args &>(*this);
        }
        
        bool is_possible(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx) {
            return !get_error(origin_card, origin, effect, ctx);
        }

        value_type random_target(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx) {
            return {};
        }

        prompt_string on_prompt(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx, value_type = {}) {
            return {};
        }

        game_string get_error(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx, value_type = {});
        void add_context(card_ptr origin_card, player_ptr origin, const effect_holder &effect, effect_context &ctx, value_type = {});
        void on_play(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx, value_type = {});
    };

    DEFINE_TARGETING(self_cubes, targeting_self_cubes)
}

#endif
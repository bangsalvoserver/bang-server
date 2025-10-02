#ifndef __TARGET_TYPE_SELECT_CUBES_H__
#define __TARGET_TYPE_SELECT_CUBES_H__

#include "cards/card_effect.h"

#include "game/possible_to_play.h"

namespace banggame {

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

        auto get_all_cubes(player_ptr origin) const {
            return cube_slots(origin) | rv::for_each([](card_ptr slot) {
                return rv::repeat(slot, slot->num_cubes());
            });
        }
        
        std::generator<card_list> possible_targets(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx);
        card_list random_target(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx);
        game_string get_error(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx, const card_list &target);
        prompt_string on_prompt(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx, const card_list &target);
        void add_context(card_ptr origin_card, player_ptr origin, const effect_holder &effect, effect_context &ctx, const card_list &target);
        void on_play(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx, const card_list &target);
    };

    DEFINE_TARGETING(select_cubes, targeting_select_cubes)
}

#endif
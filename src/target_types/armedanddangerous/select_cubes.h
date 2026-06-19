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

    namespace contexts {        
        struct selected_cubes {
            card_ptr origin_card;
            card_list cubes;

            selected_cubes(card_ptr origin_card, card_list cubes)
                : origin_card{origin_card}
                , cubes{std::move(cubes)} {}
            
            selected_cubes(card_ptr origin_card, int ncubes)
                : origin_card{origin_card}
                , cubes{static_cast<size_t>(ncubes), origin_card} {}

            static auto get_all_cubes(const effect_context &ctx) {
                return ctx.get_all<selected_cubes>() | rv::for_each(&selected_cubes::cubes);
            }

            static bool contains(const effect_context &ctx, const_card_ptr origin_card) {
                return rn::contains(ctx.get_all<selected_cubes>(), origin_card, &selected_cubes::origin_card);
            }

            static int count_repeats(const effect_context &ctx, const_card_ptr origin_card) {
                return static_cast<int>(rn::count(ctx.get_all<selected_cubes>(), origin_card, &selected_cubes::origin_card));
            }
            
            static int count_selected_on(const effect_context &ctx, const_card_ptr target_card) {
                return static_cast<int>(rn::count(get_all_cubes(ctx), target_card));
            }
        };
    }
}

#endif
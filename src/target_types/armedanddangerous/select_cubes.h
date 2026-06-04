#ifndef __TARGET_TYPE_SELECT_CUBES_H__
#define __TARGET_TYPE_SELECT_CUBES_H__

#include "cards/card_effect.h"

#include "game/possible_to_play.h"

namespace banggame {

    struct targeting_select_cubes {
        using value_type = card_list;

        int ncubes;

        targeting_select_cubes(target_args::empty, int ncubes)
            : ncubes{ncubes} {}

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
            const_card_ptr origin_card;
            card_list cubes;
            int ncubes;

            static bool contains(const effect_context &ctx, const_card_ptr origin_card) {
                for (const selected_cubes &entry : ctx.get_all<selected_cubes>()) {
                    if (entry.origin_card == origin_card && !entry.cubes.empty()) {
                        return true;
                    }
                }
                return false;
            }

            static int count_repeats(const effect_context &ctx, const_card_ptr origin_card) {
                int result = 0;
                for (const selected_cubes &entry : ctx.get_all<selected_cubes>()) {
                    if (entry.origin_card == origin_card && entry.ncubes != 0) {
                        result += entry.cubes.size() / entry.ncubes;
                    }
                }
                return result;
            }
            
            static int count_selected_on(const effect_context &ctx, const_card_ptr target_card) {
                int result = 0;
                for (const selected_cubes &entry : ctx.get_all<selected_cubes>()) {
                    for (card_ptr c : entry.cubes) {
                        if (c == target_card) ++result;
                    }
                }
                return result;
            }
        };
    }
}

#endif
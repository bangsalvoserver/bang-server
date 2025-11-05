#ifndef __TARGET_TYPE_MTH_H__
#define __TARGET_TYPE_MTH_H__

#include "cards/card_effect.h"

namespace banggame {

    struct mth_index_list {
        int count;
        std::array<int, 4> values;

        mth_index_list(std::initializer_list<int> ns) {
            auto it = values.begin();
            for (int n : ns) *it++ = n;
        }

        target_list build_targets(const target_list &targets) const {
            target_list result;
            result.reserve(count);
            for (int i=0; i<count; ++i) {
                result.push_back(targets[values[i]]);
            }
            return result;
        }
    };

    struct targeting_mth {
        using value_type = std::nullptr_t;

        mth_index_list indices;

        targeting_mth(target_args::empty = {}, std::same_as<int> auto ... indices)
            : indices{indices ...} {}
        
        bool is_possible(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx) {
            return !get_error(origin_card, origin, effect, ctx);
        }

        value_type random_target(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx) {
            return {};
        }
        
        game_string get_error(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx, value_type = {}) {
            // return effect.get_error_mth(origin_card, origin, indices.build_targets(ctx.all_targets), ctx);
            return {};
        }

        prompt_string on_prompt(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx, value_type = {}) {
            // return effect.on_prompt_mth(origin_card, origin, indices.build_targets(ctx.all_targets), ctx);
            return {};
        }

        void add_context(card_ptr origin_card, player_ptr origin, const effect_holder &effect, effect_context &ctx, value_type = {}) {
            // effect.add_context_mth(origin_card, origin, indices.build_targets(ctx.all_targets), ctx);
        }

        void on_play(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx, value_type = {}) {
            // effect.on_play_mth(origin_card, origin, indices.build_targets(ctx.all_targets), ctx);
        }
    };

    DEFINE_TARGETING(mth, targeting_mth)
}

#endif
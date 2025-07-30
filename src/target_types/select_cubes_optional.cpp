#include "select_cubes_optional.h"

#include "game/possible_to_play.h"

namespace banggame {

    card_list targeting_select_cubes_optional::random_target(const effect_context &ctx) {
        if (count_cubes(origin) >= effect.target_value) {
            return targeting_select_cubes::random_target(ctx);
        }
        return {};
    }

    game_string targeting_select_cubes_optional::get_error(const effect_context &ctx, const card_list &target_cards) {
        if (!target_cards.empty()) {
            return targeting_select_cubes::get_error(ctx, target_cards);
        }
        return {};
    }

}
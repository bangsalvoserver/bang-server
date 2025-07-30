#include "select_cubes_optional.h"

#include "game/possible_to_play.h"

namespace banggame {

    card_list targeting_select_cubes_optional::random_target(const effect_context &ctx) {
        if (count_cubes(origin) >= effect.target_value) {
            return targeting_select_cubes{*this}.random_target(ctx);
        }
        return {};
    }

    game_string targeting_select_cubes_optional::get_error(const effect_context &ctx, const card_list &target_cards) {
        if (!target_cards.empty() && target_cards.size() != effect.target_value) {
            return "ERROR_INVALID_TARGETS";
        }
        for (card_ptr c : target_cards) {
            if (c->owner != origin) {
                return {"ERROR_TARGET_NOT_SELF", origin_card};
            }
        }
        return {};
    }

}
#include "self_cubes.h"

#include "game/possible_to_play.h"

namespace banggame {

    game_string targeting_self_cubes::get_error(const effect_context &ctx, value_type) {
        if (origin_card->num_cubes() < effect.target_value) {
            return "ERROR_NOT_ENOUGH_CUBES";
        }
        return {};
    }

    void targeting_self_cubes::add_context(effect_context &ctx, value_type) {
        ctx.selected_cubes.insert(origin_card,
            card_list{static_cast<size_t>(effect.target_value), origin_card},
            effect.target_value);
    }

    void targeting_self_cubes::on_play(const effect_context &ctx, value_type) {
        origin_card->move_cubes(nullptr, effect.target_value);
    }

}
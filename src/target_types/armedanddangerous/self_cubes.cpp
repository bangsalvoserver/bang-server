#include "self_cubes.h"

#include "game/possible_to_play.h"

#include "select_cubes.h"

namespace banggame {

    game_string targeting_self_cubes::get_error(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx, value_type) {
        if (origin_card->num_cubes() < ncubes) {
            return "ERROR_NOT_ENOUGH_CUBES";
        }
        return {};
    }

    void targeting_self_cubes::add_context(card_ptr origin_card, player_ptr origin, const effect_holder &effect, effect_context &ctx, value_type) {
        ctx.add<contexts::selected_cubes>().insert(origin_card,
            card_list{static_cast<size_t>(ncubes), origin_card},
            ncubes);
    }

    void targeting_self_cubes::on_play(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx, value_type) {
        origin_card->move_cubes(nullptr, ncubes);
    }

}
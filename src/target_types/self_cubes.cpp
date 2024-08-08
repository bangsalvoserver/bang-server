#include "game/possible_to_play.h"

namespace banggame {

    using visit_cubes = play_visitor<"self_cubes">;
    
    template<> bool visit_cubes::possible(const effect_context &ctx) {
        return origin_card->num_cubes >= effect.target_value;
    }

    template<> game_string visit_cubes::get_error(const effect_context &ctx) {
        return {};
    }

    template<> game_string visit_cubes::prompt(const effect_context &ctx) {
        return {};
    }

    template<> void visit_cubes::add_context(effect_context &ctx) {
        ctx.selected_cubes.insert(origin_card,
            card_list{static_cast<size_t>(effect.target_value), origin_card},
            effect.target_value);
    }

    template<> void visit_cubes::play(const effect_context &ctx) {
        origin_card->move_cubes(nullptr, effect.target_value);
    }

}
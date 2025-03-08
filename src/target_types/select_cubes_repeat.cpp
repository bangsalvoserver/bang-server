#include "game/possible_to_play.h"

namespace banggame {

    using visit_cubes = play_visitor<"select_cubes_repeat">;

    template<> bool visit_cubes::any_of_possible_targets(const effect_context &ctx, const arg_type_predicate &fn) {
        return fn({});
    }

    template<> card_list visit_cubes::random_target(const effect_context &ctx) {
        auto cubes = origin->cube_slots()
            | rv::for_each([](card_ptr slot) {
                return rv::repeat_n(slot, slot->num_cubes());
            })
            | rn::to_vector;
        rn::shuffle(cubes, origin->m_game->bot_rng);
        
        size_t max_count = cubes.size() / effect.target_value;
        size_t num_repeats = std::uniform_int_distribution<size_t>{0, max_count}(origin->m_game->bot_rng);
        cubes.resize(effect.target_value * num_repeats);
        return cubes;
    }

    template<> game_string visit_cubes::get_error(const effect_context &ctx, const card_list &target_cards) {
        if (target_cards.size() % effect.target_value != 0) {
            return "ERROR_INVALID_TARGETS";
        }
        for (card_ptr c : target_cards) {
            if (c->owner != origin) {
                return {"ERROR_TARGET_NOT_SELF", origin_card};
            }
        }
        return {};
    }

    template<> prompt_string visit_cubes::prompt(const effect_context &ctx, const card_list &target_cards) {
        return defer<"select_cubes">().prompt(ctx, target_cards);
    }

    template<> void visit_cubes::add_context(effect_context &ctx, const card_list &target_cards) {
        defer<"select_cubes">().add_context(ctx, target_cards);
    }

    template<> void visit_cubes::play(const effect_context &ctx, const card_list &target_cards) {
        defer<"select_cubes">().play(ctx, target_cards);
    }

}
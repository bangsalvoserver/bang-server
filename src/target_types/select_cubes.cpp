#include "game/possible_to_play.h"

namespace banggame {

    using visit_cubes = play_visitor<"select_cubes">;

    template<> bool visit_cubes::possible(const effect_context &ctx) {
        return origin->count_cubes() >= effect.target_value;
    }

    template<> bool visit_cubes::any_of_possible_targets(const effect_context &ctx, const arg_type_predicate &fn) {
        return true;
    }

    template<> card_list visit_cubes::random_target(const effect_context &ctx) {
        auto cubes = origin->cube_slots()
            | rv::for_each([](card_ptr slot) {
                return rv::repeat_n(slot, slot->num_cubes());
            })
            | rn::to_vector;
        rn::shuffle(cubes, origin->m_game->bot_rng);
        
        cubes.resize(effect.target_value);
        return cubes;
    }

    template<> game_string visit_cubes::get_error(const effect_context &ctx, const card_list &target_cards) {
        if (target_cards.size() != effect.target_value) {
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
        return {};
    }

    template<> void visit_cubes::add_context(effect_context &ctx, const card_list &target_cards) {
        ctx.selected_cubes.insert(origin_card, target_cards, effect.target_value);
    }

    template<> void visit_cubes::play(const effect_context &ctx, const card_list &target_cards) {
        using cube_count_pair = std::pair<card_ptr, int>;
        std::vector<cube_count_pair> card_cube_count;

        for (card_ptr cube : target_cards) {
            auto it = rn::find(card_cube_count, cube, &cube_count_pair::first);
            if (it == card_cube_count.end()) {
                card_cube_count.emplace_back(cube, 1);
            } else {
                ++it->second;
            }
        }

        for (const auto &[cube, ncubes] : card_cube_count) {
            cube->move_cubes(nullptr, ncubes);
        }
    }

}
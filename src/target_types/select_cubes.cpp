#include "game/possible_to_play.h"

namespace banggame {

    using visit_cubes = play_visitor<"select_cubes">;

    template<> bool visit_cubes::possible(const effect_context &ctx) {
        return origin->count_cubes() >= effect.target_value;
    }

    template<> card_list visit_cubes::random_target(const effect_context &ctx) {
        auto cubes = origin->cube_slots()
            | rv::for_each([](card_ptr slot) {
                return rv::repeat_n(slot, slot->num_cubes);
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
                return "ERROR_TARGET_NOT_SELF";
            }
        }
        return {};
    }

    template<> game_string visit_cubes::prompt(const effect_context &ctx, const card_list &target_cards) {
        return {};
    }

    template<> void visit_cubes::add_context(effect_context &ctx, const card_list &target_cards) {
        for (card_ptr target : target_cards) {
            effect.add_context(origin_card, origin, target, ctx);
        }
    }

    template<> void visit_cubes::play(const effect_context &ctx, const card_list &target_cards) {
        effect.on_play(origin_card, origin, {}, ctx);
    }

}
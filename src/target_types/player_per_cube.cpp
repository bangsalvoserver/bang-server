#include "game/possible_to_play.h"

#include "effects/armedanddangerous/pay_cube.h"

namespace banggame {

    using visit_cubes = play_visitor<"player_per_cube">;

    template<> bool visit_cubes::possible(const effect_context &ctx) {
        return contains_at_least(get_all_player_targets(origin, origin_card, effect, ctx), effect.target_value);
    }

    template<> cubes_and_players visit_cubes::random_target(const effect_context &ctx) {
        auto cubes = origin->cube_slots()
            | rv::for_each([](card_ptr slot) {
                return rv::repeat_n(slot, slot->num_cubes);
            })
            | rn::to_vector;

        auto players = get_all_player_targets(origin, origin_card, effect, ctx);
        
        size_t max_count = std::min(cubes.size(), rn::distance(players) - static_cast<size_t>(effect.target_value));
        size_t ncubes = std::uniform_int_distribution<size_t>{0, max_count}(origin->m_game->bot_rng);
        
        return {
            cubes | rv::sample(ncubes) | rn::to_vector,
            players | rv::sample(ncubes + effect.target_value) | rn::to_vector
        };
    }

    template<> game_string visit_cubes::get_error(const effect_context &ctx, const cubes_and_players &target) {
        for (card_ptr c : target.first) {
            if (c->owner != origin) {
                return {"ERROR_TARGET_NOT_SELF", origin_card};
            }
        }
        if (target.first.size() + effect.target_value != target.second.size()) {
            return "ERROR_INVALID_TARGETS";
        }
        for (player_ptr target : target.second) {
            MAYBE_RETURN(defer<"player">().get_error(ctx, target));
        }
        return {};
    }

    template<> game_string visit_cubes::prompt(const effect_context &ctx, const cubes_and_players &target) {
        MAYBE_RETURN(defer<"select_cubes">().prompt(ctx, target.first));
        game_string msg;
        for (player_ptr target : target.second) {
            msg = defer<"player">().prompt(ctx, target);
            if (!msg) break;
        }
        return msg;
    }

    template<> void visit_cubes::add_context(effect_context &ctx, const cubes_and_players &target) {
        for (card_ptr target : target.first) {
            effect_pay_cube{}.add_context(origin_card, origin, target, ctx);
        }
        for (player_ptr target : target.second) {
            defer<"player">().add_context(ctx, target);
        }
    }

    template<> void visit_cubes::play(const effect_context &ctx, const cubes_and_players &target) {
        effect_pay_cube{}.on_play(origin_card, origin, ctx);
        for (player_ptr target : target.second) {
            defer<"player">().play(ctx, target);
        }
    }

}
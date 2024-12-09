#include "game/possible_to_play.h"

#include "cards/game_enums.h"

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

    template<> prompt_string visit_cubes::prompt(const effect_context &ctx, const cubes_and_players &target) {
        MAYBE_RETURN(defer<"select_cubes">().prompt(ctx, target.first));
        return merge_prompts(target.second | rv::transform([&](player_ptr target) {
            return defer<"player">().prompt(ctx, target);
        }));
    }

    template<> void visit_cubes::add_context(effect_context &ctx, const cubes_and_players &target) {
        ctx.selected_cubes.insert(origin_card, target.first, 1);
        for (player_ptr target : target.second) {
            defer<"player">().add_context(ctx, target);
        }
    }

    template<> void visit_cubes::play(const effect_context &ctx, const cubes_and_players &target) {
        defer<"select_cubes">().play(ctx, target.first);
        
        effect_flags flags = effect_flag::multi_target;
        if (target.second.size() == 1) {
            flags.add(effect_flag::single_target);
        }
        if (origin_card->is_brown()) {
            flags.add(effect_flag::escapable);
        }
        for (player_ptr target : target.second) {
            effect.on_play(origin_card, origin, target, flags, ctx);
        }
    }

}
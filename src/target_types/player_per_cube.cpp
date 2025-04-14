#include "game/possible_to_play.h"

#include "cards/game_enums.h"

#include "effects/armedanddangerous/cube_slots.h"

namespace banggame {

    using visit_cubes = play_visitor<target_types::player_per_cube>;

    template<> std::generator<target_types::player_per_cube> visit_cubes::possible_targets(const effect_context &ctx) {
        if (contains_at_least(get_all_player_targets(origin, origin_card, effect, ctx), effect.target_value)) {
            co_yield {};
        }
    }

    template<> target_types::player_per_cube visit_cubes::random_target(const effect_context &ctx) {
        auto cubes = cube_slots(origin)
            | rv::for_each([](card_ptr slot) {
                return rv::repeat_n(slot, slot->num_cubes());
            })
            | rn::to_vector;

        auto players = get_all_player_targets(origin, origin_card, effect, ctx);
        
        size_t max_count = std::min(cubes.size(), rn::distance(players) - static_cast<size_t>(effect.target_value));
        size_t ncubes = std::uniform_int_distribution<size_t>{0, max_count}(origin->m_game->bot_rng);
        
        return {
            cubes | rv::sample(ncubes, origin->m_game->bot_rng) | rn::to_vector,
            players | rv::sample(ncubes + effect.target_value, origin->m_game->bot_rng) | rn::to_vector
        };
    }

    template<> game_string visit_cubes::get_error(const effect_context &ctx, const target_types::player_per_cube &target) {
        for (card_ptr c : target.cubes) {
            if (c->owner != origin) {
                return {"ERROR_TARGET_NOT_SELF", origin_card};
            }
        }
        if (target.cubes.size() + effect.target_value != target.players.size()) {
            return "ERROR_INVALID_TARGETS";
        }
        for (player_ptr target : target.players) {
            MAYBE_RETURN(defer<target_types::player>().get_error(ctx, target));
        }
        return {};
    }

    template<> prompt_string visit_cubes::prompt(const effect_context &ctx, const target_types::player_per_cube &target) {
        MAYBE_RETURN(defer<target_types::select_cubes>().prompt(ctx, target.cubes));
        return merge_prompts(target.players | rv::transform([&](player_ptr target) {
            return defer<target_types::player>().prompt(ctx, target);
        }));
    }

    template<> void visit_cubes::add_context(effect_context &ctx, const target_types::player_per_cube &target) {
        ctx.selected_cubes.insert(origin_card, target.cubes, 1);
        for (player_ptr target : target.players) {
            defer<target_types::player>().add_context(ctx, target);
        }
    }

    template<> void visit_cubes::play(const effect_context &ctx, const target_types::player_per_cube &target) {
        defer<target_types::select_cubes>().play(ctx, target.cubes);
        
        effect_flags flags = effect_flag::multi_target;
        if (target.players.size() == 1) {
            flags.add(effect_flag::single_target);
        }
        for (player_ptr target : target.players) {
            effect.on_play(origin_card, origin, target, flags, ctx);
        }
    }

}
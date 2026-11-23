#include "player_per_cube.h"

#include "game/possible_to_play.h"
#include "game/prompts.h"

#include "cards/game_enums.h"

namespace banggame {

    static effect_flags get_flags(const targeting_player_per_cube::value_type &target) {
        effect_flags flags = effect_flag::multi_target;
        if (target.players.size() == 1) {
            flags.add(effect_flag::single_target);
        }
        return flags;
    }

    bool targeting_player_per_cube::is_possible(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx) const {
        return contains_at_least(target_player.possible_targets(origin_card, origin, effect, ctx), extra_players);
    }

    scored_target<targeting_player_per_cube::value_type> targeting_player_per_cube::random_target(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx) const {
        auto all_cubes = get_all_cubes(origin, ctx);
        
        auto players = target_player.possible_targets(origin_card, origin, effect, ctx);

        auto evaluate_fun = [&](player_ptr target) {
            if (effect.on_prompt(origin_card, origin, target, {}, ctx)) return -2;
            return effect.evaluate(origin_card, origin, target, ctx);
        };

        int count_cubes = rn::count_if(all_cubes, [](const auto &cube) { return cube.score >= 0; });
        int count_players = rn::count_if(players, [&](player_ptr p) { return evaluate_fun(p) >= 0; });

        int ncubes = count_players > extra_players ? std::min(count_cubes, count_players - extra_players) : 0;

        auto [random_cubes, cube_score] = random_max_elements(all_cubes, ncubes, origin->m_game->bot_rng, &scored_cube::score, &scored_cube::target);
        auto [random_players, score] = random_max_elements(players, ncubes + extra_players, origin->m_game->bot_rng, evaluate_fun);
        return { value_type{ std::move(random_cubes), std::move(random_players) }, score };
    }

    game_string targeting_player_per_cube::get_error(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx, const value_type &target) const {
        for (card_ptr c : target.cubes) {
            if (c->owner != origin) {
                return {"ERROR_TARGET_NOT_SELF", origin_card};
            }
        }
        if (target.cubes.size() + extra_players != target.players.size()) {
            return "ERROR_INVALID_TARGETS";
        }
        for (player_ptr target : target.players) {
            MAYBE_RETURN(target_player.get_error(origin_card, origin, effect, ctx, target));
        }
        return {};
    }

    prompt_string targeting_player_per_cube::on_prompt(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx, const value_type &target) const {
        effect_flags flags = get_flags(target);

        return prompts::select_prompt(rv::concat(
            rv::single(target_cubes.on_prompt(origin_card, origin, effect, ctx, target.cubes)),

            target.players | rv::transform([&](player_ptr target) {
                return effect.on_prompt(origin_card, origin, target, flags, ctx);
            })
        ));
    }

    void targeting_player_per_cube::add_context(card_ptr origin_card, player_ptr origin, const effect_holder &effect, effect_context &ctx, const value_type &target) const {
        target_cubes.add_context(origin_card, origin, effect, ctx, target.cubes);
        for (player_ptr target : target.players) {
            target_player.add_context(origin_card, origin, effect, ctx, target);
        }
    }

    void targeting_player_per_cube::on_play(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx, const value_type &target) const {
        effect_flags flags = get_flags(target);

        target_cubes.on_play(origin_card, origin, effect, ctx, target.cubes);
        for (player_ptr target : target.players) {
            effect.on_play(origin_card, origin, target, flags, ctx);
        }
    }

}

#include "player_per_cube.h"

#include "game/possible_to_play.h"

#include "cards/game_enums.h"

namespace banggame {

    bool targeting_player_per_cube::is_possible(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx) {
        return contains_at_least(target_player.possible_targets(origin_card, origin, effect, ctx), extra_players);
    }

    targeting_player_per_cube::value_type targeting_player_per_cube::random_target(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx) {
        auto players = target_player.possible_targets(origin_card, origin, effect, ctx);
        
        size_t max_count = std::min<size_t>(count_cubes(origin), rn::distance(players) - static_cast<size_t>(extra_players));
        size_t ncubes = std::uniform_int_distribution<size_t>{0, max_count}(origin->m_game->bot_rng);
        
        auto random_cubes = sample_elements(target_cubes.get_all_cubes(origin), ncubes, origin->m_game->bot_rng);
        auto random_players = sample_elements(players, ncubes + extra_players, origin->m_game->bot_rng);
        return { std::move(random_cubes), std::move(random_players) };
    }

    game_string targeting_player_per_cube::get_error(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx, const value_type &target) {
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

    prompt_string targeting_player_per_cube::on_prompt(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx, const value_type &target) {
        MAYBE_RETURN(target_cubes.on_prompt(origin_card, origin, effect, ctx, target.cubes));
        return merge_prompts_strict(target.players | rv::transform([&](player_ptr target) {
            return target_player.on_prompt(origin_card, origin, effect, ctx, target);
        }));
    }

    void targeting_player_per_cube::add_context(card_ptr origin_card, player_ptr origin, const effect_holder &effect, effect_context &ctx, const value_type &target) {
        target_cubes.add_context(origin_card, origin, effect, ctx, target.cubes);
        for (player_ptr target : target.players) {
            target_player.add_context(origin_card, origin, effect, ctx, target);
        }
    }

    void targeting_player_per_cube::on_play(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx, const value_type &target) {
        target_cubes.on_play(origin_card, origin, effect, ctx, target.cubes);
        
        effect_flags flags = effect_flag::multi_target;
        if (target.players.size() == 1) {
            flags.add(effect_flag::single_target);
        }
        for (player_ptr target : target.players) {
            effect.on_play(origin_card, origin, target, flags, ctx);
        }
    }

}
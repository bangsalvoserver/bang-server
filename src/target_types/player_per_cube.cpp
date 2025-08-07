#include "player_per_cube.h"

#include "select_cubes.h"
#include "player.h"

#include "game/possible_to_play.h"

#include "cards/game_enums.h"

namespace banggame {

    bool targeting_player_per_cube::is_possible(const effect_context &ctx) {
        return contains_at_least(get_all_player_targets(origin, origin_card, effect, ctx), effect.target_value);
    }

    targeting_player_per_cube::value_type targeting_player_per_cube::random_target(const effect_context &ctx) {
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

    game_string targeting_player_per_cube::get_error(const effect_context &ctx, const value_type &target) {
        for (card_ptr c : target.cubes) {
            if (c->owner != origin) {
                return {"ERROR_TARGET_NOT_SELF", origin_card};
            }
        }
        if (target.cubes.size() + effect.target_value != target.players.size()) {
            return "ERROR_INVALID_TARGETS";
        }
        for (player_ptr target : target.players) {
            MAYBE_RETURN(targeting_player{*this}.get_error(ctx, target));
        }
        return {};
    }

    prompt_string targeting_player_per_cube::on_prompt(const effect_context &ctx, const value_type &target) {
        MAYBE_RETURN(targeting_select_cubes{*this}.on_prompt(ctx, target.cubes));
        return merge_prompts(target.players | rv::transform([&](player_ptr target) {
            return targeting_player{*this}.on_prompt(ctx, target);
        }), true);
    }

    void targeting_player_per_cube::add_context(effect_context &ctx, const value_type &target) {
        ctx.selected_cubes.insert(origin_card, target.cubes, 1);
        for (player_ptr target : target.players) {
            targeting_player{*this}.add_context(ctx, target);
        }
    }

    void targeting_player_per_cube::on_play(const effect_context &ctx, const value_type &target) {
        targeting_select_cubes{*this}.on_play(ctx, target.cubes);
        
        effect_flags flags = effect_flag::multi_target;
        if (target.players.size() == 1) {
            flags.add(effect_flag::single_target);
        }
        for (player_ptr target : target.players) {
            effect.on_play(origin_card, origin, target, flags, ctx);
        }
    }

}
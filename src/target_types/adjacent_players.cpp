#include "adjacent_players.h"

#include "player.h"

#include "cards/game_enums.h"

namespace banggame {

    game_string targeting_adjacent_players::get_error(const effect_context &ctx, player_pair targets) {
        MAYBE_RETURN(check_player_filter(origin_card, origin, effect.player_filter, targets[0], ctx));
        if (!targets[1]->alive()) {
            return {"ERROR_TARGET_DEAD", origin_card, targets[1]};
        }
        if (targets[0] == targets[1] || origin->m_game->calc_distance(targets[0], targets[1]) > effect.target_value) {
            return "ERROR_TARGETS_NOT_ADJACENT";
        }
        for (player_ptr target : targets) {
            MAYBE_RETURN(effect.get_error(origin_card, origin, target, ctx));
        }
        return {};
    }

    prompt_string targeting_adjacent_players::on_prompt(const effect_context &ctx, player_pair targets) {
        return merge_prompts(targets | rv::transform([&](player_ptr target) {
            return targeting_player{*this}.on_prompt(ctx, target);
        }), true);
    }

    void targeting_adjacent_players::add_context(effect_context &ctx, player_pair targets) {
        for (player_ptr target : targets) {
            targeting_player{*this}.add_context(ctx, target);
        }
    }

    void targeting_adjacent_players::on_play(const effect_context &ctx, player_pair targets) {
        for (player_ptr target : targets) {
            effect.on_play(origin_card, origin, target, effect_flag::multi_target, ctx);
        }
    }

}
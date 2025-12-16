#include "adjacent_players.h"

#include "game/possible_to_play.h"
#include "game/prompts.h"

#include "cards/game_enums.h"

namespace banggame {

    game_string targeting_adjacent_players::get_error(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx, player_pair targets) {
        MAYBE_RETURN(check_player_filter(origin_card, origin, target_player.player_filter, targets[0], ctx));
        if (!targets[1]->alive()) {
            return {"ERROR_TARGET_DEAD", origin_card, targets[1]};
        }
        if (origin == targets[1]) {
            return "ERROR_TARGET_SELF";
        }
        if (targets[0] == targets[1] || origin->m_game->calc_distance(targets[0], targets[1]) > max_distance) {
            return "ERROR_TARGETS_NOT_ADJACENT";
        }
        for (player_ptr target : targets) {
            MAYBE_RETURN(effect.get_error(origin_card, origin, target, ctx));
        }
        return {};
    }

    prompt_string targeting_adjacent_players::on_prompt(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx, player_pair targets) {
        return prompts::select_prompt(targets | rv::transform([&](player_ptr target) {
            return target_player.on_prompt(origin_card, origin, effect, ctx, target);
        }));
    }

    void targeting_adjacent_players::add_context(card_ptr origin_card, player_ptr origin, const effect_holder &effect, effect_context &ctx, player_pair targets) {
        for (player_ptr target : targets) {
            target_player.add_context(origin_card, origin, effect, ctx, target);
        }
    }

    void targeting_adjacent_players::on_play(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx, player_pair targets) {
        for (player_ptr target : targets) {
            effect.on_play(origin_card, origin, target, effect_flag::multi_target, ctx);
        }
    }

}
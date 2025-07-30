#include "conditional_player.h"

#include "player.h"

#include "game/possible_to_play.h"

namespace banggame {

    std::generator<player_ptr> targeting_conditional_player::possible_targets(const effect_context &ctx) {
        auto targets = get_all_player_targets(origin, origin_card, effect, ctx);
        if (targets.empty()) {
            co_yield nullptr;
        } else {
            for (player_ptr target : targets) {
                co_yield target;
            }
        }
    }

    player_ptr targeting_conditional_player::random_target(const effect_context &ctx) {
        auto targets = get_all_player_targets(origin, origin_card, effect, ctx);
        if (targets.empty()) {
            return nullptr;
        } else {
            return random_element(targets, origin->m_game->bot_rng);
        }
    }

    game_string targeting_conditional_player::get_error(const effect_context &ctx, player_ptr target) {
        if (target) {
            return targeting_player{*this}.get_error(ctx, target);
        } else if (bool(get_all_player_targets(origin, origin_card, effect))) {
            return "ERROR_TARGET_SET_NOT_EMPTY";
        } else {
            return {};
        }
    }

    prompt_string targeting_conditional_player::on_prompt(const effect_context &ctx, player_ptr target) {
        if (target) {
            return targeting_player{*this}.on_prompt(ctx, target);
        } else {
            return {};
        }
    }

    void targeting_conditional_player::add_context(effect_context &ctx, player_ptr target) {
        if (target) {
            targeting_player{*this}.add_context(ctx, target);
        }
    }

    void targeting_conditional_player::on_play(const effect_context &ctx, player_ptr target) {
        if (target) {
            targeting_player{*this}.on_play(ctx, target);
        }
    }

}
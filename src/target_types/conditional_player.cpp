#include "conditional_player.h"

#include "game/possible_to_play.h"

namespace banggame {

    std::generator<player_ptr> targeting_conditional_player::possible_targets(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx) {
        if (auto targets = targeting_player::possible_targets(origin_card, origin, effect, ctx)) {
            for (player_ptr target : targets) {
                co_yield target;
            }
        } else {
            co_yield nullptr;
        }
    }

    player_ptr targeting_conditional_player::random_target(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx) {
        if (auto targets = targeting_player::possible_targets(origin_card, origin, effect, ctx)) {
            return random_element(targets, origin->m_game->bot_rng);
        } else {
            return nullptr;
        }
    }

    game_string targeting_conditional_player::get_error(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx, player_ptr target) {
        if (target) {
            return targeting_player::get_error(origin_card, origin, effect, ctx, target);
        } else if (bool(targeting_player::possible_targets(origin_card, origin, effect, ctx))) {
            return "ERROR_TARGET_SET_NOT_EMPTY";
        } else {
            return {};
        }
    }

    prompt_string targeting_conditional_player::on_prompt(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx, player_ptr target) {
        if (target) {
            return targeting_player::on_prompt(origin_card, origin, effect, ctx, target);
        } else {
            return {};
        }
    }

    void targeting_conditional_player::add_context(card_ptr origin_card, player_ptr origin, const effect_holder &effect, effect_context &ctx, player_ptr target) {
        if (target) {
            targeting_player::add_context(origin_card, origin, effect, ctx, target);
        }
    }

    void targeting_conditional_player::on_play(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx, player_ptr target) {
        if (target) {
            targeting_player::on_play(origin_card, origin, effect, ctx, target);
        }
    }

}
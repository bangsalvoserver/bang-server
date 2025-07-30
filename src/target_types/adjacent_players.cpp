#include "adjacent_players.h"

#include "player.h"

#include "game/possible_to_play.h"

#include "cards/filter_enums.h"
#include "cards/game_enums.h"

namespace banggame {

    static auto make_adjacent_players_target_set(player_ptr origin, card_ptr origin_card, const effect_holder &effect, const effect_context &ctx) {
        return rv::cartesian_product(origin->m_game->m_players, origin->m_game->m_players)
            | rv::filter([=, &effect, &ctx](const auto &pair) {
                auto [target1, target2] = pair;
                return target1 != origin && target2 != origin && target1 != target2
                    && target1->alive() && target2->alive()
                    && !check_player_filter(origin_card, origin, effect.player_filter, target1, ctx)
                    && origin->m_game->calc_distance(target1, target2) <= effect.target_value;
            });
    }

    std::generator<player_pair> targeting_adjacent_players::possible_targets(const effect_context &ctx) {
        for (auto [target1, target2] : make_adjacent_players_target_set(origin, origin_card, effect, ctx)) {
            co_yield {target1, target2};
        }
    }

    player_pair targeting_adjacent_players::random_target(const effect_context &ctx) {
        auto targets = make_adjacent_players_target_set(origin, origin_card, effect, ctx);
        auto [target1, target2] = random_element(targets, origin->m_game->bot_rng);
        return {target1, target2};
    }

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
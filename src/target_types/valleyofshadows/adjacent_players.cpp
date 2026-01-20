#include "adjacent_players.h"

#include "game/possible_to_play.h"
#include "game/prompts.h"

#include "cards/game_enums.h"

namespace banggame {

    static game_string check_second_target(card_ptr origin_card, player_ptr origin, player_ptr first_target, player_ptr second_target, const effect_context &ctx, int max_distance) {
        if (!second_target->alive()) {
            return {"ERROR_TARGET_DEAD", origin_card, second_target};
        }
        if (origin == second_target) {
            return "ERROR_TARGET_SELF";
        }
        if (first_target == second_target || origin->m_game->calc_distance(first_target, second_target) > max_distance) {
            return "ERROR_TARGETS_NOT_ADJACENT";
        }
        return {};
    }

    std::generator<player_list> targeting_adjacent_players::possible_targets(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx) {
        for (player_ptr first_target : origin->m_game->range_all_players(origin)) {
            if (!check_player_filter(origin_card, origin, target_player.player_filter, first_target, ctx)
                && !effect.get_error(origin_card, origin, first_target, ctx)
            ) {
                bool found = false;
                for (player_ptr second_target : origin->m_game->range_all_players(origin)) {
                    if (!check_second_target(origin_card, origin, first_target, second_target, ctx, max_distance)
                        && !effect.get_error(origin_card, origin, second_target, ctx)
                    ) {
                        found = true;
                        co_yield {first_target, second_target};
                    }
                }
                if (!found) {
                    co_yield {first_target};
                }
            }
        }
    }

    game_string targeting_adjacent_players::get_error(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx, const player_list &targets) {
        if (targets.empty() || targets.size() > 2) {
            return "ERROR_INVALID_TARGETS";
        }
        player_ptr first_target = targets[0];
        MAYBE_RETURN(check_player_filter(origin_card, origin, target_player.player_filter, first_target, ctx));
        if (targets.size() == 2) {
            player_ptr second_target = targets[1];
            MAYBE_RETURN(check_second_target(origin_card, origin, first_target, second_target, ctx, max_distance));
        } else if (rn::any_of(origin->m_game->range_all_players(origin), [&](player_ptr second_target) {
            return !check_second_target(origin_card, origin, first_target, second_target, ctx, max_distance);
        })) {
            return "ERROR_TARGETS_NOT_ADJACENT";
        }
        for (player_ptr target : targets) {
            MAYBE_RETURN(effect.get_error(origin_card, origin, target, ctx));
        }
        return {};
    }

    prompt_string targeting_adjacent_players::on_prompt(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx, const player_list &targets) {
        if (targets.size() == 1 && origin->m_game->num_alive() > 2) {
            return {"PROMPT_SINGLE_TARGET", origin_card};
        }
        return prompts::select_prompt(targets | rv::transform([&](player_ptr target) {
            return target_player.on_prompt(origin_card, origin, effect, ctx, target);
        }));
    }

    void targeting_adjacent_players::add_context(card_ptr origin_card, player_ptr origin, const effect_holder &effect, effect_context &ctx, const player_list &targets) {
        for (player_ptr target : targets) {
            target_player.add_context(origin_card, origin, effect, ctx, target);
        }
    }

    void targeting_adjacent_players::on_play(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx, const player_list &targets) {
        for (player_ptr target : targets) {
            effect.on_play(origin_card, origin, target, effect_flag::multi_target, ctx);
        }
    }

}
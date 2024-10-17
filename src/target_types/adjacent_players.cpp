#include "game/possible_to_play.h"

#include "cards/filter_enums.h"
#include "cards/game_enums.h"

namespace banggame {

    using visit_players = play_visitor<"adjacent_players">;

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

    template<> bool visit_players::possible(const effect_context &ctx) {
        return bool(make_adjacent_players_target_set(origin, origin_card, effect, ctx));
    }

    template<> player_list visit_players::random_target(const effect_context &ctx) {
        auto targets = make_adjacent_players_target_set(origin, origin_card, effect, ctx);
        auto [target1, target2] = random_element(targets, origin->m_game->bot_rng);
        return {target1, target2};
    }

    template<> game_string visit_players::get_error(const effect_context &ctx, const player_list &targets) {
        if (targets.size() != 2) {
            return "ERROR_INVALID_TARGETS";
        }
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

    template<> prompt_string visit_players::prompt(const effect_context &ctx, const player_list &targets) {
        return merge_prompts(targets
            | rv::transform([&](player_ptr target) { return defer<"player">().prompt(ctx, target); })
            | rn::to_vector
        );
    }

    template<> void visit_players::add_context(effect_context &ctx, const player_list &targets) {
        for (player_ptr target : targets) {
            defer<"player">().add_context(ctx, target);
        }
    }

    template<> void visit_players::play(const effect_context &ctx, const player_list &targets) {
        effect_flags flags = effect_flag::multi_target;
        if (origin_card->is_brown()) {
            flags.add(effect_flag::escapable);
        }
        for (player_ptr target : targets) {
            effect.on_play(origin_card, origin, target, flags, ctx);
        }
    }

}
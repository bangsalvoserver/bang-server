#include "game/possible_to_play.h"

namespace banggame {

    using visit_cards = play_visitor<target_types::bang_or_cards>;

    template<> std::generator<card_list> visit_cards::possible_targets(const effect_context &ctx) {
        if (get_all_card_targets(origin, origin_card, effect, ctx)) {
            co_yield {};
        }
    }

    template<> card_list visit_cards::random_target(const effect_context &ctx) {
        auto targets = get_all_card_targets(origin, origin_card, effect, ctx);
        if (auto bang_targets = targets | rv::filter([&](card_ptr target_card) {
            return target_card->is_bang_card(origin);
        })) {
            return std::vector{random_element(bang_targets, origin->m_game->bot_rng)};
        }
        return targets
            | rv::sample(effect.target_value, origin->m_game->bot_rng)
            | rn::to_vector;
    }

    template<> game_string visit_cards::get_error(const effect_context &ctx, const card_list &targets) {
        if (targets.size() == 1) {
            if (!targets.front()->is_bang_card(origin)) {
                return "ERROR_TARGET_NOT_BANG";
            }
        } else if (targets.size() != effect.target_value) {
            return "ERROR_INVALID_TARGETS";
        }
        for (card_ptr c : targets) {
            MAYBE_RETURN(defer<target_types::card>().get_error(ctx, c));
        }
        return {};
    }

    template<> prompt_string visit_cards::prompt(const effect_context &ctx, const card_list &targets) {
        return defer<target_types::cards>().prompt(ctx, targets);
    }

    template<> void visit_cards::add_context(effect_context &ctx, const card_list &targets) {
        defer<target_types::cards>().add_context(ctx, targets);
    }

    template<> void visit_cards::play(const effect_context &ctx, const card_list &targets) {
        defer<target_types::cards>().play(ctx, targets);
    }

}
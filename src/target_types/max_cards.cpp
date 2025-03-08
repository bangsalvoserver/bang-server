#include "game/possible_to_play.h"

namespace banggame {

    using visit_cards = play_visitor<"max_cards">;

    template<> std::generator<card_list> visit_cards::possible_targets(const effect_context &ctx) {
        if (get_all_card_targets(origin, origin_card, effect, ctx)) {
            co_yield {};
        }
    }

    template<> card_list visit_cards::random_target(const effect_context &ctx) {
        auto targets = get_all_card_targets(origin, origin_card, effect, ctx);
        size_t count = effect.target_value;
        if (count == 0) {
            auto dist = std::uniform_int_distribution<size_t>{1, static_cast<size_t>(rn::distance(targets))};
            count = dist(origin->m_game->bot_rng);
        }
        return targets
            | rv::sample(count, origin->m_game->bot_rng)
            | rn::to_vector;
    }

    template<> game_string visit_cards::get_error(const effect_context &ctx, const card_list &targets) {
        if (targets.empty() || effect.target_value != 0 && targets.size() > effect.target_value) {
            return "ERROR_INVALID_TARGETS";
        }
        for (card_ptr c : targets) {
            MAYBE_RETURN(defer<"card">().get_error(ctx, c));
        }
        return {};
    }

    template<> prompt_string visit_cards::prompt(const effect_context &ctx, const card_list &targets) {
        return defer<"cards">().prompt(ctx, targets);
    }

    template<> void visit_cards::add_context(effect_context &ctx, const card_list &targets) {
        defer<"cards">().add_context(ctx, targets);
    }

    template<> void visit_cards::play(const effect_context &ctx, const card_list &targets) {
        defer<"cards">().play(ctx, targets);
    }

}
#include "game/possible_to_play.h"

namespace banggame {

    using visit_cards = play_visitor<"max_cards">;

    template<> bool visit_cards::possible(const effect_context &ctx) {
        return bool(make_card_target_set(origin, origin_card, effect, ctx));
    }

    template<> serial::card_list visit_cards::random_target(const effect_context &ctx) {
        auto targets = make_card_target_set(origin, origin_card, effect, ctx);
        size_t count = effect.target_value;
        if (count == 0) {
            auto dist = std::uniform_int_distribution<size_t>{1, static_cast<size_t>(rn::distance(targets))};
            count = dist(origin->m_game->bot_rng);
        }
        return targets
            | rv::sample(count, origin->m_game->bot_rng)
            | rn::to<serial::card_list>;
    }

    template<> game_string visit_cards::get_error(const effect_context &ctx, const serial::card_list &targets) {
        if (targets.empty() || effect.target_value != 0 && targets.size() > effect.target_value) {
            return "ERROR_INVALID_TARGETS";
        }
        for (card *c : targets) {
            MAYBE_RETURN(defer<"card">().get_error(ctx, c));
        }
        return {};
    }

    template<> game_string visit_cards::prompt(const effect_context &ctx, const serial::card_list &targets) {
        return defer<"cards">().prompt(ctx, targets);
    }

    template<> void visit_cards::add_context(effect_context &ctx, const serial::card_list &targets) {
        defer<"cards">().add_context(ctx, targets);
    }

    template<> void visit_cards::play(const effect_context &ctx, const serial::card_list &targets) {
        defer<"cards">().play(ctx, targets);
    }

}
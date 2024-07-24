#include "game/play_verify.h"

#include "cards/filter_enums.h"

namespace banggame {

    using visit_cards = play_visitor<"cards">;

    template<> bool visit_cards::possible(const effect_context &ctx) {
        return contains_at_least(make_card_target_set(origin, origin_card, effect, ctx), std::max<int>(1, effect.target_value));
    }

    template<> serial::card_list visit_cards::random_target(const effect_context &ctx) {
        auto targets = make_card_target_set(origin, origin_card, effect, ctx);
        return targets
            | rv::sample(effect.target_value, origin->m_game->bot_rng)
            | rn::to<serial::card_list>;
    }

    template<> game_string visit_cards::get_error(const effect_context &ctx, const serial::card_list &targets) {
        if (targets.size() != std::max<size_t>(1, effect.target_value)) {
            return "ERROR_INVALID_TARGETS";
        }
        for (card *c : targets) {
            MAYBE_RETURN(defer<"card">().get_error(ctx, c));
        }
        return {};
    }

    template<> game_string visit_cards::prompt(const effect_context &ctx, const serial::card_list &targets) {
        for (card *c : targets) {
            MAYBE_RETURN(defer<"card">().prompt(ctx, c));
        }
        return {};
    }

    template<> void visit_cards::add_context(effect_context &ctx, const serial::card_list &targets) {
        for (card *c : targets) {
            defer<"card">().add_context(ctx, c);
        }
    }

    template<> void visit_cards::play(const effect_context &ctx, const serial::card_list &targets) {
        for (card *c : targets) {
            defer<"card">().play(ctx, c);
        }
    }

}
#include "game/possible_to_play.h"

#include "cards/filter_enums.h"

namespace banggame {

    using visit_cards = play_visitor<target_types::cards>;

    template<> std::generator<card_list> visit_cards::possible_targets(const effect_context &ctx) {
        if (contains_at_least(get_all_card_targets(origin, origin_card, effect, ctx), std::max<int>(1, effect.target_value))) {
            co_yield {};
        }
    }

    template<> card_list visit_cards::random_target(const effect_context &ctx) {
        return get_all_card_targets(origin, origin_card, effect, ctx)
            | rv::sample(effect.target_value, origin->m_game->bot_rng)
            | rn::to_vector;
    }

    template<> game_string visit_cards::get_error(const effect_context &ctx, const card_list &targets) {
        if (targets.size() != std::max<size_t>(1, effect.target_value)) {
            return "ERROR_INVALID_TARGETS";
        }
        for (card_ptr c : targets) {
            MAYBE_RETURN(defer<target_types::card>().get_error(ctx, c));
        }
        return {};
    }

    template<> prompt_string visit_cards::prompt(const effect_context &ctx, const card_list &targets) {
        return merge_prompts(targets | rv::transform([&](card_ptr c) {
            return defer<target_types::card>().prompt(ctx, c);
        }));
    }

    template<> void visit_cards::add_context(effect_context &ctx, const card_list &targets) {
        for (card_ptr c : targets) {
            defer<target_types::card>().add_context(ctx, c);
        }
    }

    template<> void visit_cards::play(const effect_context &ctx, const card_list &targets) {
        for (card_ptr c : targets) {
            defer<target_types::card>().play(ctx, c);
        }
    }

}
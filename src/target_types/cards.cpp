#include "cards.h"

#include "card.h"

#include "game/possible_to_play.h"

#include "cards/filter_enums.h"
#include "cards/game_enums.h"

namespace banggame {

    std::generator<card_list> targeting_cards::possible_targets(const effect_context &ctx) {
        if (contains_at_least(get_all_card_targets(origin, origin_card, effect, ctx), std::max<int>(1, effect.target_value))) {
            co_yield {};
        }
    }

    card_list targeting_cards::random_target(const effect_context &ctx) {
        return get_all_card_targets(origin, origin_card, effect, ctx)
            | rv::sample(effect.target_value, origin->m_game->bot_rng)
            | rn::to_vector;
    }

    game_string targeting_cards::get_error(const effect_context &ctx, const card_list &targets) {
        if (targets.size() != std::max<size_t>(1, effect.target_value)) {
            return "ERROR_INVALID_TARGETS";
        }
        for (card_ptr c : targets) {
            MAYBE_RETURN(targeting_card{*this}.get_error(ctx, c));
        }
        return {};
    }

    prompt_string targeting_cards::on_prompt(const effect_context &ctx, const card_list &targets) {
        return merge_prompts(targets | rv::transform([&](card_ptr c) {
            return targeting_card{*this}.on_prompt(ctx, c);
        }));
    }

    void targeting_cards::add_context(effect_context &ctx, const card_list &targets) {
        for (card_ptr c : targets) {
            targeting_card{*this}.add_context(ctx, c);
        }
    }

    void targeting_cards::on_play(const effect_context &ctx, const card_list &targets) {
        for (card_ptr c : targets) {
            effect.on_play(origin_card, origin, c, effect_flag::multi_target, ctx);
        }
    }

}
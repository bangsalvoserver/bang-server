#include "cards.h"

#include "card.h"

#include "cards/filter_enums.h"
#include "cards/game_enums.h"

namespace banggame {

    bool targeting_cards::is_possible(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx) {
        return contains_at_least(target_card.possible_targets(origin_card, origin, effect, ctx), ncards);
    }

    card_list targeting_cards::random_target(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx) {
        auto targets = target_card.possible_targets(origin_card, origin, effect, ctx);
        return sample_elements(targets, ncards, origin->m_game->bot_rng);
    }

    game_string targeting_cards::get_error(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx, const card_list &targets) {
        if (targets.size() != ncards) {
            return "ERROR_INVALID_TARGETS";
        }
        for (card_ptr c : targets) {
            MAYBE_RETURN(target_card.get_error(origin_card, origin, effect, ctx, c));
        }
        return {};
    }

    prompt_string targeting_cards::on_prompt(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx, const card_list &targets) {
        return merge_prompts_strict(targets | rv::transform([&](card_ptr c) {
            return target_card.on_prompt(origin_card, origin, effect, ctx, c);
        }));
    }

    void targeting_cards::add_context(card_ptr origin_card, player_ptr origin, const effect_holder &effect, effect_context &ctx, const card_list &targets) {
        for (card_ptr c : targets) {
            target_card.add_context(origin_card, origin, effect, ctx, c);
        }
    }

    void targeting_cards::on_play(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx, const card_list &targets) {
        for (card_ptr c : targets) {
            effect.on_play(origin_card, origin, c, effect_flag::multi_target, ctx);
        }
    }

}
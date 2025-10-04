#include "max_cards.h"

namespace banggame {

    bool targeting_max_cards::is_possible(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx) {
        return contains_at_least(target_card.possible_targets(origin_card, origin, effect, ctx), 1);
    }

    card_list targeting_max_cards::random_target(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx) {
        auto targets = target_card.possible_targets(origin_card, origin, effect, ctx);
        if (ncards == 0) {
            return sample_elements_streaming(targets, 0.6, origin->m_game->bot_rng);
        } else {
            return sample_elements(targets, ncards, origin->m_game->bot_rng);
        }
    }

    game_string targeting_max_cards::get_error(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx, const card_list &targets) {
        if (targets.empty() || ncards != 0 && targets.size() > ncards) {
            return "ERROR_INVALID_TARGETS";
        }
        for (card_ptr c : targets) {
            MAYBE_RETURN(target_card.get_error(origin_card, origin, effect, ctx, c));
        }
        return {};
    }

}
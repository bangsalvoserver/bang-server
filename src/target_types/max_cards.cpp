#include "max_cards.h"

#include "card.h"

#include "game/possible_to_play.h"

namespace banggame {

    bool targeting_max_cards::is_possible(const effect_context &ctx) {
        return bool(get_all_card_targets(origin, origin_card, effect, ctx));
    }

    card_list targeting_max_cards::random_target(const effect_context &ctx) {
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

    game_string targeting_max_cards::get_error(const effect_context &ctx, const card_list &targets) {
        if (targets.empty() || effect.target_value != 0 && targets.size() > effect.target_value) {
            return "ERROR_INVALID_TARGETS";
        }
        for (card_ptr c : targets) {
            MAYBE_RETURN(targeting_card{*this}.get_error(ctx, c));
        }
        return {};
    }

}
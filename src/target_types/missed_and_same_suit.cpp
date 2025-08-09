#include "missed_and_same_suit.h"

#include "cards/filter_enums.h"
#include "cards/game_enums.h"

namespace banggame {

    static bool is_missed_card(player_ptr origin, card_ptr target) {
        return origin->check_player_flags(player_flag::treat_any_as_missed)
            || target->has_tag(tag_type::missedcard)
            || (origin->check_player_flags(player_flag::treat_missed_as_bang) && target->has_tag(tag_type::bangcard));
    }

    static auto cards_with_same_suits_range(auto in_range, card_ptr first_card) {
        return rv::filter(in_range, [=](card_ptr c) {
            return c != first_card && c->sign.suit == first_card->sign.suit;
        });
    }

    static auto missed_cards_with_same_suits_range(player_ptr origin, auto in_range, short ncards) {
        return rv::filter(in_range, [=](card_ptr c) {
            return is_missed_card(origin, c)
                && contains_at_least(cards_with_same_suits_range(in_range, c), ncards - 1);
        });
    }
    
    bool targeting_missed_and_same_suit::is_possible(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx) {
        auto range = target_card.possible_targets(origin_card, origin, effect, ctx);
        return bool(missed_cards_with_same_suits_range(origin, range, ncards));
    }

    card_list targeting_missed_and_same_suit::random_target(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx) {
        auto range = target_card.possible_targets(origin_card, origin, effect, ctx);
        auto missed_cards = missed_cards_with_same_suits_range(origin, range, ncards);
        card_ptr missed_card = random_element(missed_cards, origin->m_game->bot_rng);
        auto result = cards_with_same_suits_range(range, missed_card)
            | rv::sample(ncards - 1, origin->m_game->bot_rng)
            | rn::to_vector;
        
        std::uniform_int_distribution<size_t> dist{0, result.size()};
        result.insert(result.begin() + dist(origin->m_game->bot_rng), missed_card);
        return result;
    }

    game_string targeting_missed_and_same_suit::get_error(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx, const card_list &targets) {
        MAYBE_RETURN(targeting_cards::get_error(origin_card, origin, effect, ctx, targets));

        if (rn::none_of(targets, [&](card_ptr target) {
            return is_missed_card(origin, target);
        })) {
            return "ERROR_TARGET_NOT_MISSED";
        }

        if (!rn::all_of(targets, [first_suit = targets[0]->sign.suit](card_ptr target) {
            return target->sign.suit == first_suit;
        })) {
            return "ERROR_DIFFERENT_SUITS";
        }

        return {};
    }

}
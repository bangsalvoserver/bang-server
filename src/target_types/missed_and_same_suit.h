#ifndef __TARGET_TYPE_MISSED_AND_SAME_SUIT_H__
#define __TARGET_TYPE_MISSED_AND_SAME_SUIT_H__

#include "target_types/cards.h"

namespace banggame {

    struct targeting_missed_and_same_suit : targeting_cards {
        using targeting_cards::targeting_cards;

        bool is_possible(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx);
        card_list random_target(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx);
        game_string get_error(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx, const card_list &targets);
    };

    DEFINE_TARGETING(missed_and_same_suit, targeting_missed_and_same_suit)

}

#endif
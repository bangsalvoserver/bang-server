#ifndef __TARGET_TYPE_BANG_OR_CARDS_H__
#define __TARGET_TYPE_BANG_OR_CARDS_H__

#include "cards.h"

namespace banggame {

    struct targeting_bang_or_cards : targeting_cards {
        using targeting_cards::targeting_cards;
        
        bool is_possible(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx);
        card_list random_target(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx);
        game_string get_error(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx, const card_list &target);
    };

    DEFINE_TARGETING(bang_or_cards, targeting_bang_or_cards)
}

#endif
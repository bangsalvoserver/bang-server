#ifndef __TARGET_TYPE_BANG_OR_CARDS_H__
#define __TARGET_TYPE_BANG_OR_CARDS_H__

#include "cards/card_effect.h"

#include "cards.h"

namespace banggame {

    struct targeting_bang_or_cards : targeting_cards {
        using targeting_cards::targeting_cards;
        
        std::generator<card_list> possible_targets(const effect_context &ctx);
        card_list random_target(const effect_context &ctx);
        game_string get_error(const effect_context &ctx, const card_list &target);
    };

    DEFINE_TARGETING(bang_or_cards, targeting_bang_or_cards)
}

#endif
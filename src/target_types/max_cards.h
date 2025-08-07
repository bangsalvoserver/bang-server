#ifndef __TARGET_TYPE_MAX_CARDS_H__
#define __TARGET_TYPE_MAX_CARDS_H__

#include "cards/card_effect.h"

#include "cards.h"

namespace banggame {

    struct targeting_max_cards : targeting_cards {
        using targeting_cards::targeting_cards;
        
        bool is_possible(const effect_context &ctx);
        card_list random_target(const effect_context &ctx);
        game_string get_error(const effect_context &ctx, const card_list &target);
    };

    DEFINE_TARGETING(max_cards, targeting_max_cards)
}

#endif
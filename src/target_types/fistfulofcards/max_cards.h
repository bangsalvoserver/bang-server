#ifndef __TARGET_TYPE_MAX_CARDS_H__
#define __TARGET_TYPE_MAX_CARDS_H__

#include "target_types/base/cards.h"

namespace banggame {

    struct targeting_max_cards : targeting_cards {
        targeting_max_cards(target_args::card args, int ncards = 0)
            : targeting_cards{args, ncards} {}
        
        bool is_possible(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx);
        card_list random_target(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx);
        game_string get_error(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx, const card_list &target);
    };

    DEFINE_TARGETING(max_cards, targeting_max_cards)
}

#endif
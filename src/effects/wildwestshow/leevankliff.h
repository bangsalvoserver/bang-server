#ifndef __WILDWESTSHOW_LEEVANKLIFF_H__
#define __WILDWESTSHOW_LEEVANKLIFF_H__

#include "cards/card_effect.h"

namespace banggame {

    card_ptr get_repeat_playing_card(card_ptr origin_card, const effect_context &ctx);

    struct modifier_leevankliff {
        game_string get_error(card_ptr origin_card, player_ptr origin, card_ptr target_card, const effect_context &ctx);
        void add_context(card_ptr origin_card, player_ptr origin, effect_context &ctx);
    };
    
    DEFINE_MODIFIER(leevankliff, modifier_leevankliff)
}

#endif
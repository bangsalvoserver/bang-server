#ifndef __FILTERS_H__
#define __FILTERS_H__

#include "cards/card_defs.h"

namespace banggame {
    
    game_string check_player_filter(const_card_ptr origin_card, const_player_ptr origin, enums::bitset<target_player_filter> filter, const_player_ptr target, const effect_context &ctx = {});

    game_string check_card_filter(const_card_ptr origin_card, const_player_ptr origin, enums::bitset<target_card_filter> filter, const_card_ptr target, const effect_context &ctx = {});

}

#endif
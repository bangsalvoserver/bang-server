#ifndef __FILTERS_SIMPLE_H__
#define __FILTERS_SIMPLE_H__

#include "cards/card_fwd.h"

namespace banggame::filters {

    bool is_player_bot(const_player_ptr p);

    bool is_equip_card(const_card_ptr c);

    bool is_modifier_card(const_player_ptr origin, const_card_ptr c);

    bool is_bang_card(const_player_ptr origin, const_card_ptr target);

}

#endif
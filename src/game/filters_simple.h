#ifndef __FILTERS_SIMPLE_H__
#define __FILTERS_SIMPLE_H__

#include "cards/card_fwd.h"

namespace banggame::filters {

    bool is_player_bot(const player *p);

    card *get_equip_card(card *playing_card, const effect_context &ctx);

    bool is_modifier_card(const player *origin, const card *c);

    bool is_bang_card(const player *origin, const card *target);

}

#endif
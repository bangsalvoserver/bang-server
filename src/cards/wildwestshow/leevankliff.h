#ifndef __WILDWESTSHOW_LEEVANKLIFF_H__
#define __WILDWESTSHOW_LEEVANKLIFF_H__

#include "cards/card_effect.h"

namespace banggame {

    struct modifier_leevankliff {
        game_string verify(card *origin_card, player *origin, card *playing_card);
        void add_context(card *origin_card, player *origin, effect_context &ctx);
    };
}

#endif
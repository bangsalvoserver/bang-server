#ifndef __WILDWESTSHOW_LEEVANKLIFF_H__
#define __WILDWESTSHOW_LEEVANKLIFF_H__

#include "cards/card_effect.h"

namespace banggame {

    struct effect_leevankliff {
        game_string verify(card *origin_card, player *origin);
    };
}

#endif
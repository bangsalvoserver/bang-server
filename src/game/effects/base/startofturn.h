#ifndef __BASE_STARTOFTURN_H__
#define __BASE_STARTOFTURN_H__

#include "../card_effect.h"

namespace banggame {

    struct effect_startofturn : effect_empty {
        game_string verify(card *origin_card, player *origin) const;
    };
}

#endif
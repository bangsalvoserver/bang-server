#ifndef __WILDWESTSHOW_CHANGEWWS_H__
#define __WILDWESTSHOW_CHANGEWWS_H__

#include "cards/card_effect.h"

namespace banggame {

    struct effect_changewws {
        void on_play(card *origin_card, player *origin);
    };
}

#endif
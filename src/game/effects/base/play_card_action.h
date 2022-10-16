#ifndef __BASE_PLAY_CARD_ACTION_H__
#define __BASE_COMMON_H__

#include "../card_effect.h"

namespace banggame {

    struct effect_play_card_action {
        void on_play(card *origin_card, player *origin);
    };
}

#endif
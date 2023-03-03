#ifndef __GREATTRAINROBBERY_FULL_STEAM_H__
#define __GREATTRAINROBBERY_FULL_STEAM_H__

#include "cards/card_effect.h"

namespace banggame {

    struct effect_full_steam {
        void on_play(card *origin_card, player *origin);
    };
}

#endif
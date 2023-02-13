#ifndef __ARMEDANDDANGEROUS_BANDOLIER_H__
#define __ARMEDANDDANGEROUS_BANDOLIER_H__

#include "cards/card_effect.h"

namespace banggame {

    struct modifier_bandolier {
        void add_context(card *origin_card, player *origin, effect_context &ctx);
    };

    struct effect_bandolier {
        game_string on_prompt(card *origin_card, player *origin);
        void on_play(card *origin_card, player *origin);
    };
}

#endif
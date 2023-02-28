#ifndef __ARMEDANDDANGEROUS_BANDOLIER_H__
#define __ARMEDANDDANGEROUS_BANDOLIER_H__

#include "cards/card_effect.h"
#include "cards/base/bang.h"

namespace banggame {

    struct modifier_bandolier : modifier_bangmod {
        game_string on_prompt(card *origin_card, player *origin, card *playing_card);
        void add_context(card *origin_card, player *origin, effect_context &ctx);
    };
}

#endif
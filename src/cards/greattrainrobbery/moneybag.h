#ifndef __GREATTRAINROBBERY_MONEYBAG_H__
#define __GREATTRAINROBBERY_MONEYBAG_H__

#include "cards/card_effect.h"

namespace banggame {

    struct modifier_moneybag {
        game_string get_error(card *origin_card, player *origin, card *target_card, const effect_context &ctx);
        void add_context(card *origin_card, player *origin, effect_context &ctx);
    };
}

#endif
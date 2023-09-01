#ifndef __WILDWESTSHOW_LEEVANKLIFF_H__
#define __WILDWESTSHOW_LEEVANKLIFF_H__

#include "cards/card_effect.h"

namespace banggame {

    card *get_repeat_playing_card(card *origin_card, const effect_context &ctx);

    struct modifier_leevankliff {
        game_string get_error(card *origin_card, player *origin, card *target_card, const effect_context &ctx);
        void add_context(card *origin_card, player *origin, effect_context &ctx);
    };
}

#endif
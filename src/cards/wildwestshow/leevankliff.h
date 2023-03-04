#ifndef __WILDWESTSHOW_LEEVANKLIFF_H__
#define __WILDWESTSHOW_LEEVANKLIFF_H__

#include "cards/card_effect.h"

namespace banggame {

    struct modifier_leevankliff {
        bool valid_with_equip(card *origin_card, player *origin, card *target_card) {
            return false;
        }
        bool valid_with_modifier(card *origin_card, player *origin, card *target_card) {
            return false;
        }
        game_string get_error(card *origin_card, player *origin, card *target_card);
        void add_context(card *origin_card, player *origin, effect_context &ctx);
    };
}

#endif
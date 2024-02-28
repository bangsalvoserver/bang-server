#ifndef __GREATTRAINROBBERY_SGT_BLAZE_H__
#define __GREATTRAINROBBERY_SGT_BLAZE_H__

#include "cards/card_effect.h"

namespace banggame {

    struct modifier_sgt_blaze {
        bool valid_with_equip(card *origin_card, player *origin, card *playing_card) {
            return false;
        }
        bool valid_with_modifier(card *origin_card, player *origin, card *playing_card) {
            return true;
        }
        game_string get_error(card *origin_card, player *origin, card *playing_card, const effect_context &ctx);
        void add_context(card *origin_card, player *origin, player *target, effect_context &ctx);
    };

    struct equip_sgt_blaze : event_equip {
        void on_enable(card *origin_card, player *origin);
    };

    struct handler_sgt_blaze {
        bool can_play(card *origin_card, player *origin, player *target);
        void on_play(card *origin_card, player *origin, player *target);
    };
}

#endif
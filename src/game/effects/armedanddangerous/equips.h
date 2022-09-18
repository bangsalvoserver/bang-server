#ifndef __ARMEDANDDANGEROUS_EQUIPS_H__
#define __ARMEDANDDANGEROUS_EQUIPS_H__

#include "../card_effect.h"

namespace banggame {
    
    struct effect_bomb {
        void on_equip(card *target_card, player *target);
        void on_unequip(card *target_card, player *target);
    };

    struct effect_tumbleweed : event_based_effect, effect_empty {
        void on_enable(card *target_card, player *target);
        bool can_respond(card *origin_card, player *origin);
    };

    struct handler_tumbleweed {
        void on_play(card *origin_card, player *target, opt_tagged_value<target_type::card>);
    };
}

#endif
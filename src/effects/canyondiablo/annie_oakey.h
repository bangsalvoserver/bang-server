#ifndef __CANYON_DIABLO_ANNIE_OAKEY_H__
#define __CANYON_DIABLO_ANNIE_OAKEY_H__

#include "cards/card_effect.h"

namespace banggame {
    
    struct equip_annie_oakey : event_equip {
        void on_enable(card_ptr origin_card, player_ptr origin);
    };

    DEFINE_EQUIP(annie_oakey, equip_annie_oakey)

    using card_sign_function = bool (card_sign::*)() const;

    struct effect_annie_oakey {
        card_sign_function fn;
        int ncards;
        effect_annie_oakey(card_sign_function fn, int ncards)
            : fn{fn}, ncards{ncards} {}

        bool can_play(card_ptr origin_card, player_ptr origin);
        void on_play(card_ptr origin_card, player_ptr origin);
    };

    DEFINE_EFFECT(annie_oakey, effect_annie_oakey)
}

#endif
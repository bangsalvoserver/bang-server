#ifndef __VALLEYOFSHADOWS_HENRY_BLOCK_H__
#define __VALLEYOFSHADOWS_HENRY_BLOCK_H__

#include "../card_effect.h"

namespace banggame {

    struct effect_henry_block : event_based_effect {
        void on_enable(card *target_card, player *target);
    };
}

#endif
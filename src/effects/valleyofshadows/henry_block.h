#ifndef __VALLEYOFSHADOWS_HENRY_BLOCK_H__
#define __VALLEYOFSHADOWS_HENRY_BLOCK_H__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_henry_block : event_equip {
        void on_enable(card_ptr target_card, player_ptr target);
    };

    DEFINE_EQUIP(henry_block, equip_henry_block)
}

#endif
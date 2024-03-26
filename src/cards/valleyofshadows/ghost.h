#ifndef __VALLEYOFSHADOWS_GHOST_H__
#define __VALLEYOFSHADOWS_GHOST_H__

#include "cards/card_effect.h"
#include "game/bot_suggestion.h"

namespace banggame {

    struct equip_ghost : bot_suggestion::target_friend {
        struct nodisable {};
        
        player_flags flag;
        equip_ghost(int value);
        
        void on_enable(card *target_card, player *target);
        void on_disable(card *target_card, player *target);
    };

    DEFINE_EQUIP(ghost, equip_ghost)
}

#endif
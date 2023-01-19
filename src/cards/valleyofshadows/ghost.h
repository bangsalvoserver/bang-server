#ifndef __VALLEYOFSHADOWS_GHOST_H__
#define __VALLEYOFSHADOWS_GHOST_H__

#include "cards/card_effect.h"
#include "game/bot_suggestion.h"

namespace banggame {

    struct equip_ghost : bot_suggestion::target_friend {
        void on_equip(card *target_card, player *target);
        void on_unequip(card *target_card, player *target);
    };
}

#endif
#ifndef __GHOST_CARDS_RULESET_H__
#define __GHOST_CARDS_RULESET_H__

#include "game/card.h"

namespace banggame {

    struct ruleset_ghost_cards {};

    DEFINE_RULESET(ghost_cards, ruleset_ghost_cards)

    inline bool is_ghost_card(const_card_ptr target_card) {
        return rn::contains(target_card->expansion, GET_RULESET(ghost_cards));
    }

}

#endif
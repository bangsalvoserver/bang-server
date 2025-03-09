#ifndef __BASE_SUZY_LAFAYETTE_H__
#define __BASE_SUZY_LAFAYETTE_H__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_suzy_lafayette : event_equip {
        int ncards;
        equip_suzy_lafayette(int ncards = 1): ncards{ncards} {}
        
        void on_enable(card_ptr target_card, player_ptr target);
    };

    DEFINE_EQUIP(suzy_lafayette, equip_suzy_lafayette)
}

#endif
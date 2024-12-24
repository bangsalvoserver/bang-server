#ifndef __BASE_BART_CASSIDY_H__
#define __BASE_BART_CASSIDY_H__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_bart_cassidy : event_equip {
        int ncards;
        equip_bart_cassidy(int value): ncards{std::max(1, value)} {}
        
        void on_enable(card_ptr target_card, player_ptr target);
    };

    DEFINE_EQUIP(bart_cassidy, equip_bart_cassidy)
}

#endif
#ifndef __BASE_BART_CASSIDY_H__
#define __BASE_BART_CASSIDY_H__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_bart_cassidy : event_equip {
        int ncards;
        equip_bart_cassidy(int ncards = 1): ncards{ncards} {}
        
        game_string on_prompt(card_ptr origin_card, player_ptr origin, player_ptr target) const;
        void on_enable(card_ptr target_card, player_ptr target) const;
    };

    DEFINE_EQUIP(bart_cassidy, equip_bart_cassidy)
}

#endif

#ifndef __FISTFULOFCARDS_PEYOTE_H__
#define __FISTFULOFCARDS_PEYOTE_H__

#include "cards/card_effect.h"

namespace banggame {
    
    struct equip_peyote {
        void on_enable(card_ptr target_card, player_ptr target);
        void on_disable(card_ptr target_card, player_ptr target);
    };

    DEFINE_EQUIP(peyote, equip_peyote)

    using card_sign_function = bool (card_sign::*)() const;

    struct effect_peyote {
        card_sign_function fn;
        effect_peyote(card_sign_function fn): fn{fn} {}

        bool can_play(card_ptr origin_card, player_ptr origin);
        void on_play(card_ptr origin_card, player_ptr origin);
    };

    DEFINE_EFFECT(peyote, effect_peyote)
}

#endif
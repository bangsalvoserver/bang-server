#ifndef __DODGECITY_DOC_HOLYDAY_H__
#define __DODGECITY_DOC_HOLYDAY_H__

#include "cards/card_effect.h"

namespace banggame {

    struct effect_doc_holyday {
        prompt_string on_prompt(card_ptr origin_card, player_ptr origin, const card_list &target_cards, player_ptr target);
        void on_play(card_ptr origin_card, player_ptr origin, const card_list &target_cards, player_ptr target);
    };

    DEFINE_EFFECT(doc_holyday, effect_doc_holyday)
}

#endif
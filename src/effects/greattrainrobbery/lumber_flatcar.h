#ifndef __GREATTRAINROBBERY_LUMBER_FLATCAR_H__
#define __GREATTRAINROBBERY_LUMBER_FLATCAR_H__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_lumber_flatcar {
        game_string on_prompt(card_ptr origin_card, player_ptr origin, player_ptr target);
        void on_enable(card_ptr target_card, player_ptr target);
        void on_disable(card_ptr target_card, player_ptr target);
    };

    DEFINE_EQUIP(lumber_flatcar, equip_lumber_flatcar)
}

#endif
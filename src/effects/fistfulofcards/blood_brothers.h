#ifndef __FISTFULOFCARDS_BLOOD_BROTHERS_H__
#define __FISTFULOFCARDS_BLOOD_BROTHERS_H__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_blood_brothers : event_equip {
        void on_enable(card_ptr target_card, player_ptr target) const;
    };

    DEFINE_EQUIP(blood_brothers, equip_blood_brothers)

    struct effect_blood_brothers {
        game_string get_error(card_ptr origin_card, player_ptr origin, player_ptr target) const;
        game_string on_prompt(card_ptr origin_card, player_ptr origin, player_ptr target) const;
        void on_play(card_ptr origin_card, player_ptr origin, player_ptr target) const;
    };

    DEFINE_EFFECT(blood_brothers, effect_blood_brothers)
}

#endif
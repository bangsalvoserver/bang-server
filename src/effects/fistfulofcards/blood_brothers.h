#ifndef __FISTFULOFCARDS_BLOOD_BROTHERS_H__
#define __FISTFULOFCARDS_BLOOD_BROTHERS_H__

#include "cards/card_effect.h"
#include "game/bot_suggestion.h"
#include "effects/base/prompts.h"

namespace banggame {

    struct equip_blood_brothers : event_equip {
        void on_enable(card_ptr target_card, player_ptr target);
    };

    DEFINE_EQUIP(blood_brothers, equip_blood_brothers)

    struct effect_blood_brothers : bot_suggestion::target_friend, prompt_target_ghost {
        game_string get_error(card_ptr origin_card, player_ptr origin, player_ptr target);
        void on_play(card_ptr origin_card, player_ptr origin, player_ptr target);
    };

    DEFINE_EFFECT(blood_brothers, effect_blood_brothers)
}

#endif
#ifndef __FISTFULOFCARDS_BLOOD_BROTHERS_H__
#define __FISTFULOFCARDS_BLOOD_BROTHERS_H__

#include "cards/card_effect.h"
#include "game/bot_suggestion.h"
#include "effects/base/prompts.h"

namespace banggame {

    struct equip_blood_brothers : event_equip {
        void on_enable(card *target_card, player *target);
    };

    DEFINE_EQUIP(blood_brothers, equip_blood_brothers)

    struct effect_blood_brothers : bot_suggestion::target_friend, prompt_target_ghost {
        game_string get_error(card *origin_card, player *origin, player *target);
        void on_play(card *origin_card, player *origin, player *target);
    };

    DEFINE_EFFECT(blood_brothers, effect_blood_brothers)
}

#endif
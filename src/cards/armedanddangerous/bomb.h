#ifndef __ARMEDANDDANGEROUS_BOMB_H__
#define __ARMEDANDDANGEROUS_BOMB_H__

#include "cards/card_effect.h"
#include "game/bot_suggestion.h"
#include "cards/base/prompts.h"

namespace banggame {

    struct effect_move_bomb {
        game_string on_prompt(card *origin_card, player *origin, player *target);
        game_string get_error(card *origin_card, player *origin, player *target);
        void on_play(card *origin_card, player *origin, player *target);
    };

    DEFINE_EFFECT(move_bomb, effect_move_bomb)
    
    struct equip_bomb : event_equip, prompt_target_self, bot_suggestion::target_enemy {
        void on_enable(card *target_card, player *target);
    };
}

#endif
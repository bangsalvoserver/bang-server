#ifndef __ARMEDANDDANGEROUS_BOMB_H__
#define __ARMEDANDDANGEROUS_BOMB_H__

#include "cards/card_effect.h"
#include "game/bot_suggestion.h"
#include "cards/base/prompts.h"

namespace banggame {

    struct effect_move_bomb {
        game_string on_prompt(card *origin_card, player *origin, player *target);
        game_string verify(card *origin_card, player *origin, player *target);
        void on_play(card *origin_card, player *origin, player *target);
    };
    
    struct equip_bomb : prompt_target_self, bot_suggestion::target_enemy {
        void on_equip(card *target_card, player *target);
        void on_unequip(card *target_card, player *target);
    };
}

#endif
#ifndef __ARMEDANDDANGEROUS_BOMB_H__
#define __ARMEDANDDANGEROUS_BOMB_H__

#include "cards/card_effect.h"
#include "game/bot_suggestion.h"
#include "effects/base/prompts.h"

namespace banggame {

    struct effect_move_bomb {
        game_string on_prompt(card_ptr origin_card, player_ptr origin, player_ptr target);
        game_string get_error(card_ptr origin_card, player_ptr origin, player_ptr target);
        void on_play(card_ptr origin_card, player_ptr origin, player_ptr target);
    };

    DEFINE_EFFECT(move_bomb, effect_move_bomb)
    
    struct equip_bomb : event_equip, prompt_target_self, bot_suggestion::target_enemy {
        void on_enable(card_ptr target_card, player_ptr target);
    };

    DEFINE_EQUIP(bomb, equip_bomb)
}

#endif
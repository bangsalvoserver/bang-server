#ifndef __GOLDRUSH_DISCARD_BLACK_H__
#define __GOLDRUSH_DISCARD_BLACK_H__

#include "cards/card_effect.h"
#include "effects/base/steal_destroy.h"
#include "game/bot_suggestion.h"

namespace banggame {

    struct effect_discard_black : prompt_target_self, bot_suggestion::target_enemy_card {
        game_string get_error(card_ptr origin_card, player_ptr origin, card_ptr target);
        void on_play(card_ptr origin_card, player_ptr origin, card_ptr target);
    };

    DEFINE_EFFECT(discard_black, effect_discard_black)
}

#endif
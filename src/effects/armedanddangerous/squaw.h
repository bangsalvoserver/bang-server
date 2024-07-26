#ifndef __ARMEDANDDANGEROUS_SQUAW_H__
#define __ARMEDANDDANGEROUS_SQUAW_H__

#include "cards/card_effect.h"
#include "game/bot_suggestion.h"

namespace banggame {

    struct effect_squaw : bot_suggestion::target_enemy_card {
        game_string get_error(card_ptr origin_card, player_ptr origin, card_ptr discarded_card, const effect_context &ctx);
        game_string on_prompt(card_ptr origin_card, player_ptr origin, card_ptr discarded_card, const effect_context &ctx);
        void on_play(card_ptr origin_card, player_ptr origin, card_ptr discarded_card, const effect_context &ctx);
    };

    DEFINE_EFFECT(squaw, effect_squaw)
}

#endif
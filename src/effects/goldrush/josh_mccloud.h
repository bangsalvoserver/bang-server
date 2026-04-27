#ifndef __GOLDRUSH_JOSH_MCCLOUD_H__
#define __GOLDRUSH_JOSH_MCCLOUD_H__

#include "cards/card_effect.h"

namespace banggame {

    struct effect_forced_play {
        bool can_play(card_ptr origin_card, player_ptr origin);
        void on_play(card_ptr origin_card, player_ptr origin);
    };

    DEFINE_EFFECT(forced_play, effect_forced_play)

    struct modifier_forced_play {
        game_string get_error(card_ptr origin_card, player_ptr origin, card_ptr target_card, const effect_context &ctx);
        void add_context(card_ptr origin_card, player_ptr origin, effect_context &ctx);
    };

    DEFINE_MODIFIER(forced_play, modifier_forced_play)

    struct effect_josh_mccloud {
        void on_play(card_ptr origin_card, player_ptr origin);
    };

    DEFINE_EFFECT(josh_mccloud, effect_josh_mccloud)
}

#endif
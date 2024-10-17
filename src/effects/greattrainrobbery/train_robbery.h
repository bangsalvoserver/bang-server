#ifndef __GREATTRAINROBBERY_TRAIN_ROBBERY_H__
#define __GREATTRAINROBBERY_TRAIN_ROBBERY_H__

#include "cards/card_effect.h"

namespace banggame {

    struct effect_train_robbery {
        game_string on_prompt(card_ptr origin_card, player_ptr origin, player_ptr target);
        void on_play(card_ptr origin_card, player_ptr origin, player_ptr target, effect_flags flags);
    };

    DEFINE_EFFECT(train_robbery, effect_train_robbery)

    struct effect_train_robbery_response {
        bool can_play(card_ptr origin_card, player_ptr origin);
        void add_context(card_ptr origin_card, player_ptr origin, card_ptr target, effect_context &ctx);
        void on_play(card_ptr origin_card, player_ptr origin, card_ptr target);
    };

    DEFINE_EFFECT(train_robbery_response, effect_train_robbery_response)

    struct effect_train_robbery_discard {
        game_string on_prompt(card_ptr origin_card, player_ptr origin, const effect_context &ctx);
        void on_play(card_ptr origin_card, player_ptr origin, const effect_context &ctx);
    };

    DEFINE_EFFECT(train_robbery_discard, effect_train_robbery_discard)

    struct effect_train_robbery_bang {
        game_string on_prompt(card_ptr origin_card, player_ptr origin, const effect_context &ctx);
        void on_play(card_ptr origin_card, player_ptr origin, const effect_context &ctx);
    };

    DEFINE_EFFECT(train_robbery_bang, effect_train_robbery_bang)
}

#endif
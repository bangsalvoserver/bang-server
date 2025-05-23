#ifndef __BASE_STEAL_DESTROY_H__
#define __BASE_STEAL_DESTROY_H__

#include "cards/card_effect.h"

#include "resolve.h"
#include "escapable.h"

namespace banggame {

    namespace event_type {
        struct on_destroy_card {
            player_ptr origin;
            card_ptr target_card;
            bool is_destroyed;
            nullable_ref<bool> handled;
        };
    }

    struct effect_steal {
        game_string get_error(card_ptr origin_card, player_ptr origin, card_ptr target);
        prompt_string on_prompt(card_ptr origin_card, player_ptr origin, card_ptr target);
        void on_play(card_ptr origin_card, player_ptr origin, card_ptr target, effect_flags flags = {}, const effect_context &ctx = {});
        void on_resolve(card_ptr origin_card, player_ptr origin, card_ptr target);
    };

    DEFINE_EFFECT(steal, effect_steal)

    struct effect_discard {
        bool used;
        effect_discard(bool used = false) : used{used} {}

        game_string get_error(card_ptr origin_card, player_ptr origin, card_ptr target);
        game_string on_prompt(card_ptr origin_card, player_ptr origin, card_ptr target);
        void on_play(card_ptr origin_card, player_ptr origin);
        void on_play(card_ptr origin_card, player_ptr origin, card_ptr target);
    };

    DEFINE_EFFECT(discard, effect_discard)
    
    struct effect_discard_hand {
        game_string on_prompt(card_ptr origin_card, player_ptr origin);
        void on_play(card_ptr origin_card, player_ptr origin);
    };

    DEFINE_EFFECT(discard_hand, effect_discard_hand)

    struct effect_destroy {
        prompt_string on_prompt(card_ptr origin_card, player_ptr origin, card_ptr target);
        void on_play(card_ptr origin_card, player_ptr origin, card_ptr target, effect_flags flags = {}, const effect_context &ctx = {});
        void on_resolve(card_ptr origin_card, player_ptr origin, card_ptr target);
    };

    DEFINE_EFFECT(destroy, effect_destroy)

    struct request_targeting : request_resolvable_timer {
        request_targeting(card_ptr origin_card, player_ptr origin, player_ptr target, card_ptr target_card, effect_flags flags = {}, int priority = 40)
            : request_resolvable_timer(origin_card, origin, target, flags, priority)
            , target_card(target_card) {}
        
        card_ptr target_card;

        void on_update() override;

        virtual card_list get_highlights(player_ptr owner) const override;
    };
}

#endif
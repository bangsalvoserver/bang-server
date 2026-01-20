#ifndef __BASE_STEAL_DESTROY_H__
#define __BASE_STEAL_DESTROY_H__

#include "cards/card_effect.h"

namespace banggame {

    enum class destroy_flag {
        ignore_if_dead
    };

    using destroy_flags = enums::bitset<destroy_flag>;

    namespace event_type {
        struct on_destroy_card {
            player_ptr origin;
            card_ptr target_card;
            bool is_destroyed;
            nullable_ref<destroy_flags> flags;
        };
    }

    struct effect_steal {
        game_string get_error(card_ptr origin_card, player_ptr origin, card_ptr target);
        prompt_string on_prompt(card_ptr origin_card, player_ptr origin, card_ptr target);
        void on_play(card_ptr origin_card, player_ptr origin, card_ptr target, effect_flags flags = {});
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
        void on_play(card_ptr origin_card, player_ptr origin, card_ptr target, effect_flags flags = {});
        void on_resolve(card_ptr origin_card, player_ptr origin, card_ptr target);
    };

    DEFINE_EFFECT(destroy, effect_destroy)
}

#endif
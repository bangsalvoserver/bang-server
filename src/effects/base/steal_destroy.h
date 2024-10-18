#ifndef __BASE_STEAL_DESTROY_H__
#define __BASE_STEAL_DESTROY_H__

#include "cards/card_effect.h"

#include "resolve.h"

namespace banggame {

    namespace event_type {
        struct on_destroy_card {
            player_ptr origin;
            card_ptr target_card;
            nullable_ref<bool> handled;
        };
    }

    struct effect_steal {
        game_string get_error(card_ptr origin_card, player_ptr origin, card_ptr target);
        game_string on_prompt(card_ptr origin_card, player_ptr origin, card_ptr target);
        void on_play(card_ptr origin_card, player_ptr origin, card_ptr target, effect_flags flags = {});
        void on_resolve(card_ptr origin_card, player_ptr origin, card_ptr target);
    };

    DEFINE_EFFECT(steal, effect_steal)

    struct effect_discard {
        bool used;
        effect_discard(int value = 0) : used(value) {}

        game_string get_error(card_ptr origin_card, player_ptr origin, card_ptr target);
        game_string on_prompt(card_ptr origin_card, player_ptr origin, card_ptr target);
        void on_play(card_ptr origin_card, player_ptr origin);
        void on_play(card_ptr origin_card, player_ptr origin, card_ptr target);
    };

    DEFINE_EFFECT(discard, effect_discard)

    struct effect_destroy {
        game_string on_prompt(card_ptr origin_card, player_ptr origin, card_ptr target);
        void on_play(card_ptr origin_card, player_ptr origin, card_ptr target, effect_flags flags = {});
        void on_resolve(card_ptr origin_card, player_ptr origin, card_ptr target);
    };

    DEFINE_EFFECT(destroy, effect_destroy)

    struct request_targeting : request_resolvable {
        request_targeting(card_ptr origin_card, player_ptr origin, player_ptr target, card_ptr target_card, effect_flags flags = {}, int priority = 40)
            : request_resolvable(origin_card, origin, target, flags, priority)
            , target_card(target_card) {}
        
        card_ptr target_card;

        struct timer_targeting : request_timer {
            explicit timer_targeting(request_targeting *request);
            
            void on_finished() override {
                static_cast<request_targeting *>(request)->on_resolve_target();
            }
        };

        std::optional<timer_targeting> m_timer{this};
        request_timer *timer() override { return m_timer ? &*m_timer : nullptr; }

        void on_update() override;

        void on_resolve() final;
        virtual void on_resolve_target() = 0;

        virtual card_list get_highlights() const override;
    };
}

#endif
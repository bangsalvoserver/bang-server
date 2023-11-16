#ifndef __BASE_STEAL_DESTROY_H__
#define __BASE_STEAL_DESTROY_H__

#include "cards/card_effect.h"
#include "game/bot_suggestion.h"
#include "prompts.h"

namespace banggame {

    struct effect_steal : prompt_target_self, bot_suggestion::target_enemy_card {
        game_string get_error(card *origin_card, player *origin, card *target);
        void on_play(card *origin_card, player *origin, card *target, effect_flags flags = {});
        void on_resolve(card *origin_card, player *origin, card *target);
    };

    struct effect_discard {
        bool used;
        effect_discard(int value = 0) : used(value) {}

        game_string on_prompt(card *origin_card, player *origin, card *target);
        void on_play(card *origin_card, player *origin);
        void on_play(card *origin_card, player *origin, card *target);
    };

    struct effect_destroy: prompt_target_self, bot_suggestion::target_enemy_card {
        void on_play(card *origin_card, player *origin, card *target, effect_flags flags = {});
        void on_resolve(card *origin_card, player *origin, card *target);
    };

    struct request_targeting : request_base, resolvable_request {
        request_targeting(card *origin_card, player *origin, player *target, card *target_card, effect_flags flags = {}, int priority = 0)
            : request_base(origin_card, origin, target, flags, priority)
            , target_card(target_card) {}
        
        card *target_card;

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

        virtual std::vector<card *> get_highlights() const override;
    };
}

#endif
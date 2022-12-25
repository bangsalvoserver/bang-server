#ifndef __BASE_STEAL_DESTROY_H__
#define __BASE_STEAL_DESTROY_H__

#include "cards/card_effect.h"

namespace banggame {
    
    struct prompt_target_self {
        game_string on_prompt(card *origin_card, player *origin, card *target);
    };

    struct effect_steal : prompt_target_self {
        void on_play(card *origin_card, player *origin, card *target, effect_flags flags = {});
        void on_resolve(card *origin_card, player *origin, card *target);
    };

    struct effect_discard {
        void on_play(card *origin_card, player *origin, card *target);
    };

    struct effect_destroy: prompt_target_self {
        void on_play(card *origin_card, player *origin, card *target, effect_flags flags = {});
        void on_resolve(card *origin_card, player *origin, card *target);
    };

    struct request_targeting : request_base, resolvable_request {
        request_targeting(card *origin_card, player *origin, player *target, card *target_card, effect_flags flags = {})
            : request_base(origin_card, origin, target, flags)
            , target_card(target_card) {}
        
        card *target_card;

        struct timer_targeting : request_timer {
            explicit timer_targeting(request_targeting *request);
            
            void on_finished() override {
                static_cast<request_targeting *>(request)->on_resolve_target();
            }
        };

        void on_update() override;

        void on_resolve() final;
        virtual void on_resolve_target() = 0;

        virtual std::vector<card *> get_highlights() const override;
    };
}

#endif
#ifndef __BASE_STEAL_DESTROY_H__
#define __BASE_STEAL_DESTROY_H__

#include "cards/card_effect.h"

namespace banggame {
    
    struct prompt_target_self_hand {
        game_string on_prompt(card *origin_card, player *origin, card *target);
    };

    struct effect_steal : prompt_target_self_hand {
        void on_play(card *origin_card, player *origin, card *target, effect_flags flags = {});
        void on_resolve(card *origin_card, player *origin, card *target);
    };

    struct effect_discard {
        void on_play(card *origin_card, player *origin, card *target);
    };

    struct effect_destroy: prompt_target_self_hand {
        void on_play(card *origin_card, player *origin, card *target, effect_flags flags = {});
        void on_resolve(card *origin_card, player *origin, card *target);
    };

    struct resolve_timer : request_timer {
        using request_timer::request_timer;
        void on_finished() override;
    };

    struct request_targeting : request_base, resolvable_request {
        request_targeting(card *origin_card, player *origin, player *target, card *target_card, effect_flags flags = {})
            : request_base(origin_card, origin, target, flags | effect_flags::auto_respond_empty_hand)
            , target_card(target_card) {}
        
        card *target_card;

        resolve_timer m_timer{this};
        request_timer *timer() override { return &m_timer; }

        void on_resolve() final;
        virtual void on_resolve_target() = 0;

        virtual std::vector<card *> get_highlights() const override;
    };

    inline void resolve_timer::on_finished() {
        static_cast<request_targeting *>(request)->on_resolve_target();
    }
}

#endif
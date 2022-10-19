#ifndef __BASE_STEAL_DESTROY_H__
#define __BASE_STEAL_DESTROY_H__

#include "../card_effect.h"

namespace banggame {
    
    struct prompt_target_self_hand {
        game_string on_prompt(card *origin_card, player *origin, card *target);
    };

    struct effect_steal : prompt_target_self_hand {
        void on_play(card *origin_card, player *origin, card *target, effect_flags flags = {});
        void on_resolve(card *origin_card, player *origin, card *target);
    };

    struct effect_discard {
        void on_play(card *origin_card, player *origin, card *target, effect_flags flags = {});
        void on_resolve(card *origin_card, player *origin, card *target);
    };

    struct effect_destroy : effect_discard, prompt_target_self_hand {};

    struct request_targeting : request_base {
        request_targeting(card *origin_card, player *origin, player *target, card *target_card, effect_flags flags = {})
            : request_base(origin_card, origin, target, flags | effect_flags::auto_respond_empty_hand)
            , target_card(target_card) {}
        
        card *target_card;

        virtual std::vector<card *> get_highlights() const override;
    };
}

#endif
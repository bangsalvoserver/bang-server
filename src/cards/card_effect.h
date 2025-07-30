#ifndef __CARD_EFFECT_H__
#define __CARD_EFFECT_H__

#include "card_defs.h"

#include "game/request_base.h"

#include "vtables.h"

namespace banggame {

    struct event_equip {
        void on_disable(card_ptr target_card, player_ptr target);
    };

    struct targeting_base {
        card_ptr origin_card;
        player_ptr origin;
        const effect_holder &effect;

        targeting_base(card_ptr origin_card, player_ptr origin, const effect_holder &effect)
            : origin_card{origin_card}, origin{origin}, effect{effect} {}
        
        template<std::derived_from<targeting_base> T>
        targeting_base(const T &value)
            : targeting_base{value.origin_card, value.origin, value.effect} {}
    };

}


#endif
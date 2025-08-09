#ifndef __CARD_EFFECT_H__
#define __CARD_EFFECT_H__

#include "card_defs.h"

#include "game/request_base.h"

#include "vtables.h"

namespace banggame {

    struct event_equip {
        void on_disable(card_ptr target_card, player_ptr target);
    };

    enum class target_filter {
        none,
        player,
        card
    };

    template<typename T, target_filter F>
    struct targeting_args;

    template<> struct targeting_args<void, target_filter::none>{};

    template<> struct targeting_args<void, target_filter::player>{
        player_filter_bitset player_filter;
    };

    template<> struct targeting_args<void, target_filter::card>{
        player_filter_bitset player_filter;
        card_filter_bitset card_filter;
    };

    template<typename T> struct targeting_args<T, target_filter::none>{
        T target_value;
    };

    template<typename T> struct targeting_args<T, target_filter::player>{
        T target_value;
        player_filter_bitset player_filter;
    };

    template<typename T> struct targeting_args<T, target_filter::card>{
        T target_value;
        player_filter_bitset player_filter;
        card_filter_bitset card_filter;
    };

}


#endif
#ifndef __PLAY_VISITOR_H__
#define __PLAY_VISITOR_H__

#include "play_verify.h"
#include "player.h"
#include "game.h"

namespace banggame {

    template<typename T> struct const_ref_if_non_trivial {
        using type = const T &;
    };

    template<typename T> requires std::is_trivially_copyable_v<T> struct const_ref_if_non_trivial<T> {
        using type = T;
    };

    template<target_type E> struct play_visitor {
        game_string verify(const play_card_verify *verifier, const effect_holder &effect);
        game_string prompt(const play_card_verify *verifier, const effect_holder &efffect);
        void play(const play_card_verify *verifier, const effect_holder &holder);
    };

    template<target_type E> requires (play_card_target::has_type<E>)
    struct play_visitor<E> {
        using arg_type = typename const_ref_if_non_trivial<typename play_card_target::value_type<E>>::type;

        game_string verify(const play_card_verify *verifier, const effect_holder &effect, arg_type arg);
        game_string prompt(const play_card_verify *verifier, const effect_holder &efffect, arg_type arg);
        void play(const play_card_verify *verifier, const effect_holder &holder, arg_type arg);
    };

}

#endif
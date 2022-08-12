#ifndef __PLAY_VISITOR_H__
#define __PLAY_VISITOR_H__

#include "play_verify.h"

#include "player.h"
#include "game.h"

#define OPT_ARG(...) OPT_ARG_1(,##__VA_ARGS__, OPT_ARG_2(__VA_ARGS__),)
#define OPT_ARG_1(arg1, arg2, arg3, ...) arg3
#define OPT_ARG_2(...) , __VA_ARGS__

namespace banggame {

    template<target_type E> struct play_visitor;

    #define DECLARE_VISITOR(type, ...) \
    template<> struct play_visitor<target_type::type> { \
        opt_error verify(const play_card_verify *verifier, const effect_holder &effect OPT_ARG(__VA_ARGS__)); \
        opt_fmt_str prompt(const play_card_verify *verifier, const effect_holder &effect OPT_ARG(__VA_ARGS__)); \
        void play(const play_card_verify *verifier, const effect_holder &effect OPT_ARG(__VA_ARGS__)); \
    };

    DECLARE_VISITOR(none)
    DECLARE_VISITOR(player, player *target)
    DECLARE_VISITOR(conditional_player, nullable<player> target)
    DECLARE_VISITOR(card, card *target)
    DECLARE_VISITOR(extra_card, nullable<card>)
    DECLARE_VISITOR(all_players)
    DECLARE_VISITOR(other_players)
    DECLARE_VISITOR(cards_other_players, const std::vector<card *> &target_cards)
    DECLARE_VISITOR(cube, const std::vector<card *> &target_cards)

    static_assert([]<target_type ... Es>(enums::enum_sequence<Es ...>){
        return (requires { play_visitor<Es>{}; } && ... );
    }(enums::make_enum_sequence<target_type>()),
        "All visitors for target_type must be defined");

}

#endif
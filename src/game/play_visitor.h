#ifndef __PLAY_VISITOR_H__
#define __PLAY_VISITOR_H__

#include "play_verify.h"
#include "player.h"
#include "game.h"

namespace banggame {

    using player_set = std::set<player *>;
    using card_set = std::set<card *>;
    using card_cube_count = std::map<card *, int>;
    using duplicate_set = std::variant<std::monostate, player_set, card_set, card_cube_count>;

    template<target_type E> struct play_visitor {
        const play_card_verify &verifier;
        const effect_holder &effect;

        game_string verify();
        duplicate_set duplicates();
        game_string prompt();
        void play();
    };

    template<target_type E> requires (play_card_target::has_type<E>)
    struct play_visitor<E> {
        using arg_type = same_if_trivial_t<unwrap_not_null_t<typename play_card_target::value_type<E>>>;

        const play_card_verify &verifier;
        const effect_holder &effect;

        game_string verify(arg_type arg);
        duplicate_set duplicates(arg_type arg);
        game_string prompt(arg_type arg);
        void play(arg_type arg);
    };

}

#endif
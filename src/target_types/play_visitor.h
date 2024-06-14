#ifndef __PLAY_VISITOR_H__
#define __PLAY_VISITOR_H__

#include "cards/card_defs.h"

namespace banggame {

    template<target_type E> struct play_visitor {
        player *origin;
        card *origin_card;
        const effect_holder &effect;

        template<target_type T>
        play_visitor<T> defer() const {
            return {origin, origin_card, effect};
        }

        bool possible(const effect_context &ctx);
        game_string get_error(const effect_context &ctx);
        game_string prompt(const effect_context &ctx);
        void add_context(effect_context &ctx);
        void play(const effect_context &ctx);
    };

    template<target_type E> requires (play_card_target::has_type<E>)
    struct play_visitor<E> {
        using value_type = unwrap_not_null_t<typename play_card_target::value_type<E>>;
        using arg_type = same_if_trivial_t<value_type>;

        player *origin;
        card *origin_card;
        const effect_holder &effect;

        template<target_type T>
        play_visitor<T> defer() const {
            return {origin, origin_card, effect};
        }

        bool possible(const effect_context &ctx);
        value_type random_target(const effect_context &ctx);
        game_string get_error(const effect_context &ctx, arg_type arg);
        game_string prompt(const effect_context &ctx, arg_type arg);
        void add_context(effect_context &ctx, arg_type arg);
        void play(const effect_context &ctx, arg_type arg);
    };
}

#endif
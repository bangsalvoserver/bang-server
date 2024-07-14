#ifndef __PLAY_VISITOR_H__
#define __PLAY_VISITOR_H__

#include "cards/card_defs.h"

namespace banggame {
    
    template<utils::tstring Name>
    using target_type_value = utils::tagged_variant_value_type<play_card_target, Name>;

    template<typename T> struct same_if_trivial { using type = const T &; };
    template<typename T> using same_if_trivial_t = typename same_if_trivial<T>::type;

    template<typename T> requires (std::is_trivially_copyable_v<T>)
    struct same_if_trivial<T> { using type = T; };

    template<utils::tstring Name> struct play_visitor {
        static_assert(std::is_void_v<target_type_value<Name>>);
        
        player *origin;
        card *origin_card;
        const effect_holder &effect;

        template<utils::tstring E>
        play_visitor<E> defer() const {
            return {origin, origin_card, effect};
        }

        bool possible(const effect_context &ctx);
        game_string get_error(const effect_context &ctx);
        game_string prompt(const effect_context &ctx);
        void add_context(effect_context &ctx);
        void play(const effect_context &ctx);
    };

    template<utils::tstring Name> requires (!std::is_void_v<target_type_value<Name>>)
    struct play_visitor<Name> {
        using value_type = unwrap_not_null_t<target_type_value<Name>>;
        using arg_type = same_if_trivial_t<value_type>;

        player *origin;
        card *origin_card;
        const effect_holder &effect;

        template<utils::tstring E>
        play_visitor<E> defer() const {
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
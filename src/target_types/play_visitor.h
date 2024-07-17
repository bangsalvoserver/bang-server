#ifndef __PLAY_VISITOR_H__
#define __PLAY_VISITOR_H__

#include "cards/card_defs.h"

namespace banggame {

    template<typename T>
    concept target_type_tag = utils::tag_for<T, play_card_target>;
    
    template<target_type_tag Tag>
    using target_type_value = utils::tagged_variant_value_type<play_card_target, Tag>;

    template<target_type_tag Tag> struct play_visitor_t {
        player *origin;
        card *origin_card;
        const effect_holder &effect;

        template<utils::fixed_string E>
        play_visitor_t<utils::tag<E>> defer() const {
            return {origin, origin_card, effect};
        }

        bool possible(const effect_context &ctx);
        std::monostate random_target(const effect_context &ctx) { return {}; }
        game_string get_error(const effect_context &ctx);
        game_string prompt(const effect_context &ctx);
        void add_context(effect_context &ctx);
        void play(const effect_context &ctx);
    };
    
    template<typename T> struct unwrap_not_null { using type = T; };
    template<typename T> struct unwrap_not_null<not_null<T *>> { using type = T *; };

    template<target_type_tag Tag> requires (!std::is_void_v<target_type_value<Tag>>)
    struct play_visitor_t<Tag> {
        using value_type = typename unwrap_not_null<target_type_value<Tag>>::type;
        using arg_type = std::conditional_t<std::is_trivially_copyable_v<value_type>, value_type, const value_type &>;

        player *origin;
        card *origin_card;
        const effect_holder &effect;

        template<utils::fixed_string E>
        play_visitor_t<utils::tag<E>> defer() const {
            return {origin, origin_card, effect};
        }

        bool possible(const effect_context &ctx);
        value_type random_target(const effect_context &ctx);
        game_string get_error(const effect_context &ctx, arg_type arg);
        game_string prompt(const effect_context &ctx, arg_type arg);
        void add_context(effect_context &ctx, arg_type arg);
        void play(const effect_context &ctx, arg_type arg);
    };

    template<utils::fixed_string Name>
    using play_visitor = play_visitor_t<utils::tag<Name>>;
}

#endif
#ifndef __PLAY_DISPATCH_H__
#define __PLAY_DISPATCH_H__

#include "cards/card_defs.h"

namespace banggame::play_dispatch {

    bool possible(player_ptr origin, card_ptr origin_card, const effect_holder &effect, const effect_context &ctx);

    play_card_target random_target(player_ptr origin, card_ptr origin_card, const effect_holder &effect, const effect_context &ctx);

    game_string get_error(player_ptr origin, card_ptr origin_card, const effect_holder &effect, const effect_context &ctx, const play_card_target &target);

    prompt_string prompt(player_ptr origin, card_ptr origin_card, const effect_holder &effect, const effect_context &ctx, const play_card_target &target);

    void add_context(player_ptr origin, card_ptr origin_card, const effect_holder &effect, effect_context &ctx, const play_card_target &target);

    void play(player_ptr origin, card_ptr origin_card, const effect_holder &effect, const effect_context &ctx, const play_card_target &target);

}

namespace banggame {

    template<typename T>
    concept target_type_tag = utils::tag_for<T, play_card_target>;
    
    template<target_type_tag Tag>
    using target_type_value = utils::tagged_variant_value_type<play_card_target, Tag>;

    template<target_type_tag Tag> struct play_visitor_t {
        player_ptr origin;
        card_ptr origin_card;
        const effect_holder &effect;

        template<utils::fixed_string E>
        play_visitor_t<utils::tag<E>> defer() const {
            return {origin, origin_card, effect};
        }

        bool possible(const effect_context &ctx);
        std::monostate random_target(const effect_context &ctx) { return {}; }
        game_string get_error(const effect_context &ctx);
        prompt_string prompt(const effect_context &ctx);
        void add_context(effect_context &ctx);
        void play(const effect_context &ctx);
    };

    template<target_type_tag Tag> requires (!std::is_void_v<target_type_value<Tag>>)
    struct play_visitor_t<Tag> {
        using value_type = target_type_value<Tag>;
        using arg_type = std::conditional_t<std::is_trivially_copyable_v<value_type>, value_type, const value_type &>;

        player_ptr origin;
        card_ptr origin_card;
        const effect_holder &effect;

        template<utils::fixed_string E>
        play_visitor_t<utils::tag<E>> defer() const {
            return {origin, origin_card, effect};
        }

        bool possible(const effect_context &ctx);
        value_type random_target(const effect_context &ctx);
        game_string get_error(const effect_context &ctx, arg_type arg);
        prompt_string prompt(const effect_context &ctx, arg_type arg);
        void add_context(effect_context &ctx, arg_type arg);
        void play(const effect_context &ctx, arg_type arg);
    };

    template<utils::fixed_string Name>
    using play_visitor = play_visitor_t<utils::tag<Name>>;
}

#endif
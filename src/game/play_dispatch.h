#ifndef __PLAY_DISPATCH_H__
#define __PLAY_DISPATCH_H__

#include "cards/card_defs.h"

#include <generator>

namespace banggame::play_dispatch {

    std::generator<play_card_target> possible_targets(player_ptr origin, card_ptr origin_card, const effect_holder &effect, const effect_context &ctx);

    play_card_target random_target(player_ptr origin, card_ptr origin_card, const effect_holder &effect, const effect_context &ctx);

    game_string get_error(player_ptr origin, card_ptr origin_card, const effect_holder &effect, const effect_context &ctx, const play_card_target &target);

    prompt_string prompt(player_ptr origin, card_ptr origin_card, const effect_holder &effect, const effect_context &ctx, const play_card_target &target);

    void add_context(player_ptr origin, card_ptr origin_card, const effect_holder &effect, effect_context &ctx, const play_card_target &target);

    void play(player_ptr origin, card_ptr origin_card, const effect_holder &effect, const effect_context &ctx, const play_card_target &target);

}

namespace banggame {

    template<typename T>
    decltype(auto) unwrap_target(const T &value) {
        if constexpr (reflect::size<T>() == 1) {
            const auto &[ unwrapped ] = value;
            return unwrapped;
        } else {
            return value;
        }
    }

    template<typename T> struct play_visitor {
        using value_type = std::remove_cvref_t<decltype(unwrap_target(std::declval<T>()))>;
        using arg_type = std::conditional_t<std::is_trivially_copyable_v<value_type>, value_type, const value_type &>;

        player_ptr origin;
        card_ptr origin_card;
        const effect_holder &effect;

        template<typename U> play_visitor<U> defer() const {
            return {origin, origin_card, effect};
        }

        std::generator<value_type> possible_targets(const effect_context &ctx);
        value_type random_target(const effect_context &ctx);
        game_string get_error(const effect_context &ctx, arg_type arg);
        prompt_string prompt(const effect_context &ctx, arg_type arg);
        void add_context(effect_context &ctx, arg_type arg);
        void play(const effect_context &ctx, arg_type arg);
    };
}

#endif
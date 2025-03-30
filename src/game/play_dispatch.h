#ifndef __PLAY_DISPATCH_H__
#define __PLAY_DISPATCH_H__

#include "cards/card_defs.h"
#include "utils/json_aggregate.h"

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
    concept empty_target = requires {
        requires json::aggregate<T>;
        requires reflect::size<T>() == 0;
    };

    template<typename T> struct play_visitor;
    
    template<empty_target T> struct play_visitor<T> {
        using value_type = T;

        player_ptr origin;
        card_ptr origin_card;
        const effect_holder &effect;

        template<typename U> play_visitor<U> defer() const {
            return {origin, origin_card, effect};
        }

        std::generator<value_type> possible_targets(const effect_context &ctx);
        value_type random_target(const effect_context &ctx) { return {}; }
        game_string get_error(const effect_context &ctx);
        prompt_string prompt(const effect_context &ctx);
        void add_context(effect_context &ctx);
        void play(const effect_context &ctx);
    };

    template<json::transparent_aggregate T> struct play_visitor<T> {
        using value_type = json::transparent_value_type<T>;
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
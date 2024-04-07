#ifndef __PLAY_VERIFY_H__
#define __PLAY_VERIFY_H__

#include "possible_to_play.h"

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

    DEFINE_ENUM_TYPES(message_type,
        (ok)
        (error, game_string)
        (prompt, game_string)
    )

    using game_message = enums::enum_variant<message_type>;

    game_string get_play_card_error(player *origin, card *origin_card, const effect_context &ctx);

    game_string get_equip_error(player *origin, card *origin_card, player *target, const effect_context &ctx);

    void apply_target_list(player *origin, card *origin_card, bool is_response, const target_list &targets, const effect_context &ctx);

    void apply_add_context(player *origin, card *origin_card, const effect_holder &effect, const play_card_target &target, effect_context &ctx);

    game_message verify_and_play(player *origin, const game_action &action);

}

#endif